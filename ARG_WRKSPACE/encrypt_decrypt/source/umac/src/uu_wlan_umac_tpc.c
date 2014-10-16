/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_umac_tpc.c                                            **
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include "uu_wlan_umac_tpc.h"
#include "uu_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_main.h"
#include "uu_wlan_rate.h"

#include "uu_wlan_buf_desc.h"

#ifdef UU_WLAN_TPC

/** Definitions */
uu_uint8 TPC_Link_Min_Inc;
uu_uint8 TPC_Link_Max_Dec;
uu_int8 uu_wlan_transmit_power_dBm[UU_WLAN_TPLevelMax];

uu_uint8 TPC_Max_Bandwidth;
uu_uint8 dot11TxPowerLevelExtended;

uu_uint8 uu_wlan_lowpload_datamgmt_ltpl[CBW80_80 * UU_WLAN_AC_MAX_AC];
uu_uint8 uu_wlan_highpload_datamgmt_ltpl[CBW80_80 * UU_WLAN_AC_MAX_AC];
uu_uint8 uu_wlan_firstframe_multiprot [CBW80_80];

/** Initializing static variables */
#define PATH_LOSS_EWMA_LEVEL    20
static int path_loss_value = -1;
static int path_loss_range = -1;

/**
 * Initializing table for cumulative TFAR,
 * Wideband TFAR, Wideband AC TFAR */
int wactfar[CBW80_80 * UU_WLAN_AC_MAX_AC] = {0};
int wtfar[CBW80_80] = {0};
int tfar = 0;

/**
 * Initializing local variables used for
 * cumulative TFAR, Wideband TFAR, Wideband AC TFAR */
#define EWMA_LEVEL  25
static int attempt_hist = 0;
static int failed_hist = 0;
static int curr_prob = 0;

/** Initializing the tpc request timer */
uu_int32 TPC_Request_Timer_Interval = UU_REG_LMAC_TPCREQ_INRVL;


/**-------------------------------------------------------------------------------------
 * ---------------------INITIALIZE TPC TABLES AND DEFAULT-------------------------------
 * -------------------------------------------------------------------------------------
 */

/** Initializing Transmit power values (dBm) as per the transmit power levels for 2 GHz */
static uu_void uu_wlan_init_power_tpl_2GHz (uu_void)
{
    /** This is for 802.11b/g/n. 802.11b reuires 4 TPL,
     * 802.11g/n requires 8 TPL.
     * This should be in mW, and should not cross 100 mW.
     * l2 < l1 < n2 < n1 < m2 < m1 < h2 < h1
     */
                                                            /* mW */   /* TPL */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel0] = 17;      /* 50 */     /* l2 */

    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel1] = 7;       /* 5;  */   /* l2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel2] = 10;      /* 10; */   /* l1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel3] = 13;      /* 20; */   /* n2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel4] = 14;      /* 25; */   /* n1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel5] = 17;      /* 50; */   /* m2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel6] = 20;      /* 100; */   /* m1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel7] = 23;      /* 200; */   /* h2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel8] = 30;      /* 1000; */  /* h1 */
}

/** Initializing Transmit power values (dBm) as per the transmit power levels for 5 GHz */
static uu_void uu_wlan_init_power_tpl_5GHz (uu_void)
{
    /** This is for 802.11a/n/ac. 802.11a/n reuires 8 TPL,
     * 802.11ac requires 128 TPL.
     * For 802.11a/n units should be in mW and
     * for 802.11ac units should be in 250 uW.
     * For our implementation, we will be using first 8
     * for 802.11a/n and the remaining 120 for 802.11ac
     */
                                                            /* 250 uW */   /* TPL */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel0] = 17;      /* 200 */     /* l2 */

    /** For the first 8 TPL will be defined in this way */
    /** l2 < l1 < n2 < n1 < m2 < m1 < h2 < h1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel1] = 4;       /* 10 */    /* l2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel2] = 7;       /* 20 */    /* l1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel3] = 9;       /* 32 */    /* n2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel4] = 11;      /* 50 */    /* n1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel5] = 13;      /* 80 */    /* m2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel6] = 15;      /* 126 */   /* m1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel7] = 21;      /* 503 */   /* h2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel8] = 36;      /* 16000 */ /* h1 */

    /** For the remaining 120, TPL will be in increasing order of transmit power,
       and will be used only for 802.11ac */
    /** Transmit Power Level l2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel9] = 0;       /* 4 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel10] = 0;      /* 4 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel11] = 0;      /* 4 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel12] = 1;      /* 5 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel13] = 1;      /* 5 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel14] = 2;      /* 7 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel15] = 2;      /* 7 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel16] = 2;      /* 7 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel17] = 3;      /* 8 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel18] = 3;      /* 8 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel19] = 3;      /* 8 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel20] = 3;      /* 8 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel21] = 3;      /* 8 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel22] = 4;      /* 10 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel23] = 4;      /* 10 */
    /* Transmit Power Level l1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel24] = 4;      /* 10 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel25] = 4;      /* 10 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel26] = 5;      /* 12 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel27] = 5;      /* 12 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel28] = 5;      /* 12 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel29] = 5;      /* 12 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel30] = 5;      /* 12 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel31] = 6;      /* 16 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel32] = 6;      /* 16 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel33] = 6;      /* 16 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel34] = 6;      /* 16 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel35] = 6;      /* 16 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel36] = 6;      /* 16 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel37] = 7;      /* 20 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel38] = 7;      /* 20 */
    /* Transmit Power Level n2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel39] = 7;      /* 20 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel40] = 7;      /* 20 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel41] = 7;      /* 20 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel42] = 7;      /* 20 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel43] = 7;      /* 20 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel44] = 7;      /* 20 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel45] = 8;      /* 24 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel46] = 8;      /* 24 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel47] = 8;      /* 24 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel48] = 8;      /* 24 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel49] = 8;      /* 24 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel50] = 8;      /* 24 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel51] = 8;      /* 24 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel52] = 9;      /* 32 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel53] = 9;      /* 32 */
    /* Transmit Power Level n1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel54] = 9;      /* 32 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel55] = 9;      /* 32 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel56] = 9;      /* 32 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel57] = 9;      /* 32 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel58] = 9;      /* 32 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel59] = 9;      /* 32 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel60] = 10;     /* 40 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel61] = 10;     /* 40 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel62] = 10;     /* 40 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel63] = 10;     /* 40 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel64] = 10;     /* 40 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel65] = 10;     /* 40 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel66] = 10;     /* 40 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel67] = 10;     /* 40 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel68] = 11;     /* 50 */
    /* Transmit Power Level m2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel69] = 11;     /* 50 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel70] = 11;     /* 50 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel71] = 11;     /* 50 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel72] = 11;     /* 50 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel73] = 11;     /* 50 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel74] = 11;     /* 50 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel75] = 11;     /* 50 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel76] = 12;     /* 63 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel77] = 12;     /* 63 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel78] = 12;     /* 63 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel79] = 12;     /* 63 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel80] = 12;     /* 63 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel81] = 12;     /* 63 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel82] = 12;     /* 63 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel83] = 13;     /* 80 */
    /* Transmit Power Level m1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel84] = 13;     /* 80 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel85] = 13;     /* 80 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel86] = 13;     /* 80 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel87] = 13;     /* 80 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel88] = 13;     /* 80 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel89] = 13;     /* 80 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel90] = 14;     /* 100 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel91] = 14;     /* 100 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel92] = 14;     /* 100 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel93] = 14;     /* 100 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel94] = 14;     /* 100 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel95] = 14;     /* 100 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel96] = 15;     /* 126 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel97] = 15;     /* 126 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel98] = 15;     /* 126 */
    /* Transmit Power Level h2 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel99] = 15;     /* 126 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel100] = 15;    /* 126 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel101] = 15;    /* 126 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel102] = 15;    /* 126 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel103] = 16;    /* 160 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel104] = 16;    /* 160 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel105] = 16;    /* 160 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel106] = 16;    /* 160 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel107] = 16;    /* 160 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel108] = 17;    /* 200 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel109] = 17;    /* 200 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel110] = 18;    /* 252 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel111] = 19;    /* 320 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel112] = 20;    /* 400 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel113] = 21;    /* 504 */
    /* Transmit Power Level h1 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel114] = 22;    /* 640 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel115] = 23;    /* 800 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel116] = 24;    /* 1008 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel117] = 25;    /* 1280 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel118] = 26;    /* 1600 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel119] = 27;    /* 2016 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel120] = 28;    /* 2560 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel121] = 29;    /* 3200 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel122] = 30;    /* 4000 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel123] = 31;    /* 5120 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel124] = 32;    /* 6400 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel125] = 33;    /* 8000 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel126] = 34;    /* 10440 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel127] = 35;    /* 12648 */
    uu_wlan_transmit_power_dBm[UU_WLAN_TPLevel128] = 36;    /* 16000 */
}

/** Lower limit for data/management frames with payload < RTS_Threshold
 *
 * bk20, bk40, bk80, bk160, be20, be40, be80, be160,
 * vi20, vi49, vi80, vi160, vo20, vo40, vo80, vo160
 *
 *  Priority/Bandwidth      Background      Best Effort     Video       Voice
 *
 *      20 MHz              l2 (lowest)     l1 (lowest)     n2 (lowest) n2 (lowest)
 *      40 MHz              l1 (lowest)     n2 (lowest)     n1 (lowest) n1 (lowest)
 *      80 MHz              n2 (lowest)     n2 (lowest)     n1 (lowest) m2 (lowest)
 *      160 MHz             n1 (lowest)     n1 (lowest)     m2 (lowest) m2 (lowest)
 */
static uu_void uu_wlan_init_lowpload_datamgmt_ltpl_2GHz (uu_void)
{
    uu_wlan_lowpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel4;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel3;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel2;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel1;

    uu_wlan_lowpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel5;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel4;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel3;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel2;
}
static uu_void uu_wlan_init_lowpload_datamgmt_ltpl_5GHz (uu_void)
{
    uu_wlan_lowpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel39;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel29;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel19;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel9;

    uu_wlan_lowpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel54;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel44;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel34;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel24;

    uu_wlan_lowpload_datamgmt_ltpl[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel69;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel59;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel49;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel39;

    uu_wlan_lowpload_datamgmt_ltpl[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel84;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel74;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel64;
    uu_wlan_lowpload_datamgmt_ltpl[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel54;
}


/** Lower limit for data/management frames with payload >= RTS_Threshold
 *
 *  Priority/Bandwidth      Background      Best Effort     Video       Voice
 *
 *      20 MHz              n2 (lowest)     n2 (lowest)     n1 (lowest) n1 (lowest)
 *      40 MHz              n1 (lowest)     n1 (lowest)     m2 (lowest) m2 (lowest)
 *      80 MHz              m2 (lowest)     m2 (lowest)     m1 (lowest) m1 (lowest)
 *      160 MHz             m1 (lowest)     m1 (lowest)     h2 (lowest) h2 (lowest)
 */
static uu_void uu_wlan_init_highpload_datamgmt_ltpl_2GHz (uu_void)
{
    uu_wlan_highpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel4;
    uu_wlan_highpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel4;
    uu_wlan_highpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel3;
    uu_wlan_highpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel3;

    uu_wlan_highpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel5;
    uu_wlan_highpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel5;
    uu_wlan_highpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel4;
    uu_wlan_highpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel4;
}
static uu_void uu_wlan_init_highpload_datamgmt_ltpl_5GHz (uu_void)
{
    uu_wlan_highpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel54;
    uu_wlan_highpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel49;
    uu_wlan_highpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel44;
    uu_wlan_highpload_datamgmt_ltpl[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel39;

    uu_wlan_highpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel69;
    uu_wlan_highpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel64;
    uu_wlan_highpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel59;
    uu_wlan_highpload_datamgmt_ltpl[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel54;

    uu_wlan_highpload_datamgmt_ltpl[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel84;
    uu_wlan_highpload_datamgmt_ltpl[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel79;
    uu_wlan_highpload_datamgmt_ltpl[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel74;
    uu_wlan_highpload_datamgmt_ltpl[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel69;

    uu_wlan_highpload_datamgmt_ltpl[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel99;
    uu_wlan_highpload_datamgmt_ltpl[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel94;
    uu_wlan_highpload_datamgmt_ltpl[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel89;
    uu_wlan_highpload_datamgmt_ltpl[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel84;
}

/** Lower limit for data/management frames that are first frame in multiple protection or
 * if duration < TXOP_limit and the frame where pending frames increases, i.e., duration increases.
 *
 *  Priority/Bandwidth      TPC for the 1
 *
 *       20 MHz              m1 (lowest)
 *       40 MHz              m1 (lowest)
 *       80 MHz              h2 (lowest)
 *       160 MHz             h2 (lowest)
 */
static uu_void uu_wlan_init_firstframe_multiprot_2GHz (uu_void)
{
    uu_wlan_firstframe_multiprot[CBW20] = UU_WLAN_TPLevel6;
    uu_wlan_firstframe_multiprot[CBW40] = UU_WLAN_TPLevel7;
}
static uu_void uu_wlan_init_firstframe_multiprot_5GHz (uu_void)
{
    uu_wlan_firstframe_multiprot[CBW20] = UU_WLAN_TPLevel84;
    uu_wlan_firstframe_multiprot[CBW40] = UU_WLAN_TPLevel89;
    uu_wlan_firstframe_multiprot[CBW80] = UU_WLAN_TPLevel94;
    uu_wlan_firstframe_multiprot[CBW160] = UU_WLAN_TPLevel99;
}


/** Defining Base TPC Level Table.
 *
 *  For AP
 *  Priority/Bandwidth  Background  Best Effort     Video   Voice
 *
 *      20 MHz              h2 (0)      h2 (2)      h2 (4)  h2 (6)
 *      40 MHz              h2 (4)      h2 (6)      h2 (8)  h2 (10)
 *      80 MHz              h2 (8)      h2 (10)     h2 (12) h2 (14)
 *      160 MHz             h2 (12)     h2 (14)     h1 (1)  h1 (3)
 *
 * For STA
 *  Priority/Bandwidth  Background  Best Effort     Video   Voice
 *
 *      20 MHz              m2 (0)      m2 (2)      m2 (4)  m2 (6)
 *      40 MHz              m2 (4)      m2 (6)      m2 (8)  m2 (10)
 *      80 MHz              m2 (8)      m2 (10)     m2 (12) m2 (14)
 *      160 MHz             m2 (12)     m2 (14)     m1 (1)  m1 (3)
 */
static uu_void uu_wlan_init_basetpl_2GHz (uu_void)
{
    if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
    {
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel7;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel7;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel6;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel6;

        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel7;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel7;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel6;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel6;
    }
    else
    {
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel5;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel5;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel4;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel4;

        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel5;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel5;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel4;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel4;
    }
} /* uu_wlan_init_basetpl_2GHz  */

static uu_void uu_wlan_init_basetpl_5GHz (uu_void)
{
    if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
    {
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel105;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel103;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel101;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel99;

        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel109;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel107;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel105;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel103;

        uu_wlan_btpl_ac_g[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel113;
        uu_wlan_btpl_ac_g[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel111;
        uu_wlan_btpl_ac_g[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel109;
        uu_wlan_btpl_ac_g[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel107;

        uu_wlan_btpl_ac_g[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel117;
        uu_wlan_btpl_ac_g[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel115;
        uu_wlan_btpl_ac_g[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel113;
        uu_wlan_btpl_ac_g[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel111;
    }
    else
    {
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel75;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel73;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel71;
        uu_wlan_btpl_ac_g[(CBW20 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel69;

        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel79;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel77;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel75;
        uu_wlan_btpl_ac_g[(CBW40 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel73;

        uu_wlan_btpl_ac_g[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel83;
        uu_wlan_btpl_ac_g[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel81;
        uu_wlan_btpl_ac_g[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel79;
        uu_wlan_btpl_ac_g[(CBW80 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel77;

        uu_wlan_btpl_ac_g[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO] = UU_WLAN_TPLevel87;
        uu_wlan_btpl_ac_g[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VI] = UU_WLAN_TPLevel85;
        uu_wlan_btpl_ac_g[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BK] = UU_WLAN_TPLevel83;
        uu_wlan_btpl_ac_g[(CBW160 * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_BE] = UU_WLAN_TPLevel81;
    }
} /* uu_wlan_init_basetpl_5GHz  */


/** Initiatize the tables and parameters for 2 GHz */
uu_void uu_wlan_tpc_levels_init_2GHz (uu_void)
{
    /** Initialize TPC vs. TPL */
    uu_wlan_init_power_tpl_2GHz ();

    /** Initialize lower limits for data/management frames with payload < RTS_Threshold */
    uu_wlan_init_lowpload_datamgmt_ltpl_2GHz ();

    /** Initialize lower limits for data/management frames with payload >= RTS_Threshold */
    uu_wlan_init_highpload_datamgmt_ltpl_2GHz ();

    /** Initialize lower limits for the first frame in multiple protection */
    uu_wlan_init_firstframe_multiprot_2GHz ();

    /** Initialize base_TPL for data/management frames */
    uu_wlan_init_basetpl_2GHz ();

    /** Level difference (l2, l1, n2, n1, m2, m1, h2, h1) */
    uu_wlan_tp_level_diff_g = 1;
    uu_wlan_retry_ofdm_tpl_inc_g = 1;

    TPC_Max_Bandwidth = CBW40;
} /* uu_wlan_tpc_levels_init_2GHz */

/** Initiatize the tables and parameters for 5 GHz */
uu_void uu_wlan_tpc_levels_init_5GHz (uu_void)
{
    /** Initialize TPC vs. TPL */
    uu_wlan_init_power_tpl_5GHz ();

    /** Initialize lower limits for data/management frames with payload < RTS_Threshold */
    uu_wlan_init_lowpload_datamgmt_ltpl_5GHz ();

    /** Initialize lower limits for data/management frames with payload >= RTS_Threshold */
    uu_wlan_init_highpload_datamgmt_ltpl_5GHz ();

    /** Initialize lower limits for the first frame in multiple protection */
    uu_wlan_init_firstframe_multiprot_5GHz ();

    /** Initialize base_TPL for data/management frames */
    uu_wlan_init_basetpl_5GHz ();

    /** Level difference (l2, l1, n2, n1, m2, m1, h2, h1) */
    uu_wlan_tp_level_diff_g = 15;
    uu_wlan_retry_11ac_tpl_inc_g = 15;
    uu_wlan_retry_ofdm_tpl_inc_g = 1;

    TPC_Max_Bandwidth = CBW160;
} /* uu_wlan_tpc_levels_init_5GHz */

/**-------------------------------------------------------------------------------------
 * ---------------------Updating Upper Threshold for TPL--------------------------------
 * -------------------------------------------------------------------------------------
 */

/** This function will convert the transmit power in dBm to the corresponding TP Level */
static uu_uint8 uu_wlan_dBm_to_TPLevel (int tpc_dBm, uu_uint8 utpl_min, uu_uint8 utpl_max)
{
    uu_uint8 ret_TPL;

    ret_TPL = utpl_max;

    /** Now, determine the threshold as per the max_power dBm */
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("uu_wlan_transmit_power_dBm[utpl_max] : %d\n", uu_wlan_transmit_power_dBm[ret_TPL]);
#endif
    if (tpc_dBm < uu_wlan_transmit_power_dBm[ret_TPL])
    {
        while (uu_wlan_transmit_power_dBm[ret_TPL] >= tpc_dBm)
        {
            if (ret_TPL > utpl_min)
            {
                if ((uu_wlan_transmit_power_dBm[ret_TPL - 1] < tpc_dBm) && (uu_wlan_transmit_power_dBm[ret_TPL] == tpc_dBm))
                {
                    break;
                }
                ret_TPL --;
            }
            else
            {
                break;
            }
        }
    }

    return ret_TPL;
} /* uu_wlan_dBm_to_TPLevel */

/**
 * Define utpl_threshold whenever max_power changes.
 * This function will define the upper TPL threshold
 * for OFDM TPL and VHT TPL corresponding to the bandwidth
 */

uu_void uu_wlan_configure_utpl_threshold (int tpc_dBm, uu_uint8 bandwidth)
{
    uu_uint8 utpl_min;
    uu_uint8 utpl_max;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("TPC_DBM: %d\n", tpc_dBm);
#endif
    /* Determine the maximum value as per the supported in device */
    utpl_min = UU_WLAN_TPLevel1;
    utpl_max = dot11NumberSupportedPowerLevelsImplemented_g;

    uu_wlan_utpl_threshold_11n_g[bandwidth] = uu_wlan_dBm_to_TPLevel (tpc_dBm, utpl_min, utpl_max);

    if (uu_wlan_btpl_ac_g[0] >= UU_WLAN_TPLevel9)
    {
        utpl_min = UU_WLAN_TPLevel9;
        utpl_max = dot11TxPowerLevelExtended;

        uu_wlan_utpl_threshold_11ac_g[bandwidth] = uu_wlan_dBm_to_TPLevel (tpc_dBm, utpl_min, utpl_max);
    }
} /* uu_wlan_configure_utpl_threshold  */


/**-------------------------------------------------------------------------------------
 * ---------------------HANDLING TPC REQUEST/REPORT FRAME-------------------------------
 * -------------------------------------------------------------------------------------
 */

/** RSSI Power Values with respect to RSSI Level.
 * FIXME: This is PHY-DEPENDENT, and change this table as per the device support.
 * This should be in dBm
 */
uu_int8 uu_wlan_rssi_level_to_rssi_power[UU_WLAN_RSSI_MAXIMUM] =
{
    /** Refer to: P802.11ac D3.0, Pg 186, RSSI.
     * RSSI is intended to be used in a relative manner, and it is a monotonically
     * increasing function of the received power.
     */
    -128,   -127,   -127,   -126,   -126,   -125,   -125,   -124,   -124,   -123,
    -123,   -122,   -122,   -121,   -121,   -120,   -120,   -119,   -119,   -118,
    -118,   -117,   -117,   -116,   -116,   -115,   -115,   -114,   -114,   -113,
    -113,   -112,   -112,   -111,   -111,   -110,   -110,   -109,   -109,   -108,
    -108,   -107,   -107,   -106,   -106,   -105,   -105,   -104,   -104,   -103,
    -103,   -102,   -102,   -101,   -101,   -100,   -100,   -99,    -99,    -98,
    -98,    -97,    -97,    -96,    -96,    -95,    -95,    -94,    -94,    -93,
    -93,    -92,    -92,    -91,    -91,    -90,    -90,    -89,    -89,    -88,
    -88,    -87,    -87,    -86,    -86,    -85,    -85,    -84,    -84,    -83,
    -83,    -82,    -82,    -81,    -81,    -80,    -80,    -79,    -79,    -78,
    -78,    -77,    -77,    -76,    -76,    -75,    -75,    -74,    -74,    -73,
    -73,    -72,    -72,    -71,    -71,    -70,    -70,    -69,    -69,    -68,
    -68,    -67,    -67,    -66,    -66,    -65,    -65,    -64,    -64,    -63,
    -63,    -62,    -62,    -61,    -61,    -60,    -60,    -59,    -59,    -58,
    -58,    -57,    -57,    -56,    -56,    -55,    -55,    -54,    -54,    -53,
    -53,    -52,    -52,    -51,    -51,    -50,    -50,    -49,    -49,    -48,
    -48,    -47,    -47,    -46,    -46,    -45,    -45,    -44,    -44,    -43,
    -43,    -42,    -42,    -41,    -41,    -40,    -40,    -39,    -39,    -38,
    -38,    -37,    -37,    -36,    -36,    -35,    -35,    -34,    -34,    -33,
    -33,    -32,    -32,    -31,    -31,    -30,    -30,    -29,    -29,    -28,
    -28,    -27,    -27,    -26,    -26,    -25,    -25,    -24,    -24,    -23,
    -23,    -22,    -22,    -21,    -21,    -20,    -20,    -19,    -19,    -18,
    -18,    -17,    -17,    -16,    -16,    -15,    -15,    -14,    -14,    -13,
    -13,    -12,    -12,    -11,    -11,    -10,    -10,    -9,     -9,     -8,
    -8,     -7,     -7,     -6,     -6,     -5,     -5,     -4,     -4,     -3
    -3,     -2,     -2,     -1,     -1,     0,
};


/** As per MCS and Nsts, it will vary */
const uu_mcs_rec_min_sensitivity_table_t uu_wlan_recv_min_sensitivity_table_mcs_g[UU_MAX_STREAM_VHT][UU_VHT_MCS_INDEX_MAXIMUM] =
{
    /* mcs_20_bcc_lgi                   mcs_20_ldpc_lgi                     mcs_20_bcc_sgi                      mcs_20_ldpc_sgi
            mcs_40_bcc_lgi                  mcs_40_ldpc_lgi                     mcs_40_bcc_sgi                      mcs_40_ldpc_sgi
                mcs_80_bcc_lgi                  mcs_80_ldpc_lgi                     mcs_80_bcc_sgi                      mcs_80_ldpc_sgi
                    mcs_160_bcc_lgi                 mcs_160_ldpc_lgi                    mcs_160_bcc_sgi                      mcs_160_ldpc_sgi
    */
    /** For Nsts = 1 */
    {
        {-82,   -79,    -76,    -73,        -79,   -76,    -73,    -70,         -82,   -79,    -76,    -73,         -79,   -76,    -73,    -70,},
        {-79,   -76,    -73,    -70,        -77,   -74,    -71,    -68,         -79,   -76,    -73,    -70,         -77,   -74,    -71,    -68,},
        {-77,   -74,    -71,    -68,        -74,   -71,    -68,    -65,         -77,   -74,    -71,    -68,         -74,   -71,    -68,    -65,},
        {-74,   -71,    -68,    -65,        -70,   -67,    -64,    -61,         -74,   -71,    -68,    -65,         -70,   -67,    -64,    -61,},
        {-70,   -67,    -64,    -61,        -66,   -63,    -60,    -57,         -70,   -67,    -64,    -61,         -66,   -63,    -60,    -57,},
        {-66,   -63,    -60,    -57,        -65,   -62,    -59,    -56,         -66,   -63,    -60,    -57,         -65,   -62,    -59,    -56,},
        {-65,   -62,    -59,    -56,        -64,   -61,    -58,    -55,         -65,   -62,    -59,    -56,         -64,   -61,    -58,    -55,},
        {-64,   -61,    -58,    -55,        -59,   -56,    -53,    -50,         -64,   -61,    -58,    -55,         -59,   -56,    -53,    -50,},
        {-59,   -56,    -53,    -50,        -57,   -54,    -51,    -48,         -59,   -56,    -53,    -50,         -57,   -54,    -51,    -48,},
        {-57,   -54,    -51,    -48,        -54,   -51,    -48,    -45,         -57,   -54,    -51,    -48,         -54,   -51,    -48,    -45,},
    },

    /** For Nsts = 2 */
    {
        {-77,   -74,    -71,    -68,        -74,   -71,    -68,    -65,         -77,   -74,    -71,    -68,         -74,   -71,    -68,    -65,},
        {-74,   -71,    -68,    -65,        -70,   -67,    -64,    -61,         -74,   -71,    -68,    -65,         -70,   -67,    -64,    -61,},
        {-70,   -67,    -64,    -61,        -66,   -63,    -60,    -57,         -70,   -67,    -64,    -61,         -66,   -63,    -60,    -57,},
        {-66,   -63,    -60,    -57,        -65,   -62,    -59,    -56,         -66,   -63,    -60,    -57,         -65,   -62,    -59,    -56,},
        {-65,   -62,    -59,    -56,        -64,   -61,    -58,    -55,         -65,   -62,    -59,    -56,         -64,   -61,    -58,    -55,},
        {-64,   -61,    -58,    -55,        -59,   -56,    -53,    -50,         -64,   -61,    -58,    -55,         -59,   -56,    -53,    -50,},
        {-59,   -56,    -53,    -50,        -57,   -54,    -51,    -48,         -59,   -56,    -53,    -50,         -57,   -54,    -51,    -48,},
        {-57,   -54,    -51,    -48,        -54,   -51,    -48,    -45,         -57,   -54,    -51,    -48,         -54,   -51,    -48,    -45,},
        {-54,   -51,    -48,    -45,        -51,   -48,    -45,    -42,         -54,   -51,    -48,    -45,         -51,   -48,    -45,    -42,},
        {-51,   -48,    -45,    -42,        -48,   -45,    -42,    -39,         -51,   -48,    -45,    -42,         -48,   -45,    -42,    -39,},
    },

    /** For Nsts = 3 */
    {
        {-70,   -67,    -64,    -61,        -66,   -63,    -60,    -57,         -70,   -67,    -64,    -61,         -66,   -63,    -60,    -57,},
        {-66,   -63,    -60,    -57,        -65,   -62,    -59,    -56,         -66,   -63,    -60,    -57,         -65,   -62,    -59,    -56,},
        {-65,   -62,    -59,    -56,        -64,   -61,    -58,    -55,         -65,   -62,    -59,    -56,         -64,   -61,    -58,    -55,},
        {-64,   -61,    -58,    -55,        -59,   -56,    -53,    -50,         -64,   -61,    -58,    -55,         -59,   -56,    -53,    -50,},
        {-59,   -56,    -53,    -50,        -57,   -54,    -51,    -48,         -59,   -56,    -53,    -50,         -57,   -54,    -51,    -48,},
        {-57,   -54,    -51,    -48,        -54,   -51,    -48,    -45,         -57,   -54,    -51,    -48,         -54,   -51,    -48,    -45,},
        {-54,   -51,    -48,    -45,        -51,   -48,    -45,    -42,         -54,   -51,    -48,    -45,         -51,   -48,    -45,    -42,},
        {-51,   -48,    -45,    -42,        -48,   -45,    -42,    -39,         -51,   -48,    -45,    -42,         -48,   -45,    -42,    -39,},
        {-48,   -45,    -42,    -39,        -45,   -42,    -39,    -36,         -48,   -45,    -42,    -39,         -45,   -42,    -39,    -36,},
        {-45,   -42,    -39,    -36,        -42,   -39,    -36,    -33,         -45,   -42,    -39,    -36,         -42,   -39,    -36,    -33,},
    },

    /** For Nsts = 4 */
    {
        {-65,   -62,    -59,    -56,        -64,   -61,    -58,    -55,         -65,   -62,    -59,    -56,         -64,   -61,    -58,    -55,},
        {-64,   -61,    -58,    -55,        -59,   -56,    -53,    -50,         -64,   -61,    -58,    -55,         -59,   -56,    -53,    -50,},
        {-59,   -56,    -53,    -50,        -57,   -54,    -51,    -48,         -59,   -56,    -53,    -50,         -57,   -54,    -51,    -48,},
        {-57,   -54,    -51,    -48,        -54,   -51,    -48,    -45,         -57,   -54,    -51,    -48,         -54,   -51,    -48,    -45,},
        {-54,   -51,    -48,    -45,        -51,   -48,    -45,    -42,         -54,   -51,    -48,    -45,         -51,   -48,    -45,    -42,},
        {-51,   -48,    -45,    -42,        -48,   -45,    -42,    -39,         -51,   -48,    -45,    -42,         -48,   -45,    -42,    -39,},
        {-48,   -45,    -42,    -39,        -45,   -42,    -39,    -36,         -48,   -45,    -42,    -39,         -45,   -42,    -39,    -36,},
        {-45,   -42,    -39,    -36,        -42,   -39,    -36,    -33,         -45,   -42,    -39,    -36,         -42,   -39,    -36,    -33,},
        {-42,   -39,    -36,    -33,        -39,   -36,    -33,    -30,         -42,   -39,    -36,    -33,         -42,   -39,    -33,    -30,},
        {-39,   -36,    -33,    -30,        -36,   -33,    -30,    -27,         -39,   -36,    -33,    -30,         -36,   -33,    -30,    -27,},
    },

    /** For Nsts = 5 */
    {
        {-59,   -56,    -53,    -50,        -57,   -54,    -51,    -48,         -59,   -56,    -53,    -50,         -57,   -54,    -51,    -48,},
        {-57,   -54,    -51,    -48,        -54,   -51,    -48,    -45,         -57,   -54,    -51,    -48,         -54,   -51,    -48,    -45,},
        {-54,   -51,    -48,    -45,        -51,   -48,    -45,    -42,         -54,   -51,    -48,    -45,         -51,   -48,    -45,    -42,},
        {-51,   -48,    -45,    -42,        -48,   -45,    -42,    -39,         -51,   -48,    -45,    -42,         -48,   -45,    -42,    -39,},
        {-48,   -45,    -42,    -39,        -45,   -42,    -39,    -36,         -48,   -45,    -42,    -39,         -45,   -42,    -39,    -36,},
        {-45,   -42,    -39,    -36,        -42,   -39,    -36,    -33,         -45,   -42,    -39,    -36,         -42,   -39,    -36,    -33,},
        {-42,   -39,    -36,    -33,        -39,   -36,    -33,    -30,         -42,   -39,    -36,    -33,         -42,   -39,    -33,    -30,},
        {-39,   -36,    -33,    -30,        -36,   -33,    -30,    -27,         -39,   -36,    -33,    -30,         -36,   -33,    -30,    -27,},
        {-36,   -33,    -30,    -27,        -33,   -30,    -27,    -24,         -36,   -33,    -30,    -27,         -33,   -30,    -27,    -24,},
        {-33,   -30,    -27,    -24,        -30,   -27,    -24,    -21,         -33,   -30,    -27,    -24,         -30,   -27,    -24,    -21,},
    },

    /** For Nsts = 6 */
    {
        {-54,   -51,    -48,    -45,        -51,   -48,    -45,    -42,         -54,   -51,    -48,    -45,         -51,   -48,    -45,    -42,},
        {-51,   -48,    -45,    -42,        -48,   -45,    -42,    -39,         -51,   -48,    -45,    -42,         -48,   -45,    -42,    -39,},
        {-48,   -45,    -42,    -39,        -45,   -42,    -39,    -36,         -48,   -45,    -42,    -39,         -45,   -42,    -39,    -36,},
        {-45,   -42,    -39,    -36,        -42,   -39,    -36,    -33,         -45,   -42,    -39,    -36,         -42,   -39,    -36,    -33,},
        {-42,   -39,    -36,    -33,        -39,   -36,    -33,    -30,         -42,   -39,    -36,    -33,         -42,   -39,    -33,    -30,},
        {-39,   -36,    -33,    -30,        -36,   -33,    -30,    -27,         -39,   -36,    -33,    -30,         -36,   -33,    -30,    -27,},
        {-36,   -33,    -30,    -27,        -33,   -30,    -27,    -24,         -36,   -33,    -30,    -27,         -33,   -30,    -27,    -24,},
        {-33,   -30,    -27,    -24,        -30,   -27,    -24,    -21,         -33,   -30,    -27,    -24,         -30,   -27,    -24,    -21,},
        {-30,   -27,    -24,    -21,        -27,   -24,    -21,    -18,         -30,   -27,    -24,    -21,         -27,   -24,    -21,    -18,},
        {-27,   -24,    -21,    -18,        -24,   -21,    -18,    -15,         -27,   -24,    -21,    -18,         -24,   -21,    -18,    -15,},
    },

    /** For Nsts = 7 */
    {
        {-48,   -45,    -42,    -39,        -45,   -42,    -39,    -36,         -48,   -45,    -42,    -39,         -45,   -42,    -39,    -36,},
        {-45,   -42,    -39,    -36,        -42,   -39,    -36,    -33,         -45,   -42,    -39,    -36,         -42,   -39,    -36,    -33,},
        {-42,   -39,    -36,    -33,        -39,   -36,    -33,    -30,         -42,   -39,    -36,    -33,         -42,   -39,    -33,    -30,},
        {-39,   -36,    -33,    -30,        -36,   -33,    -30,    -27,         -39,   -36,    -33,    -30,         -36,   -33,    -30,    -27,},
        {-36,   -33,    -30,    -27,        -33,   -30,    -27,    -24,         -36,   -33,    -30,    -27,         -33,   -30,    -27,    -24,},
        {-33,   -30,    -27,    -24,        -30,   -27,    -24,    -21,         -33,   -30,    -27,    -24,         -30,   -27,    -24,    -21,},
        {-30,   -27,    -24,    -21,        -27,   -24     -21,    -18,         -30,   -27,    -24,    -21,         -27,   -24     -21,    -18,},
        {-27,   -24,    -21,    -18,        -24,   -21,    -18,    -15,         -27,   -24,    -21,    -18,         -24,   -21,    -18,    -15,},
        {-24,   -21,    -18,    -15,        -21,   -18,    -15,    -12,         -24,   -21,    -18,    -15,         -21,   -18,    -15,    -12,},
        {-21,   -18,    -15,    -12,        -18,   -15,    -12,    -9,          -21,   -18,    -15,    -12,         -18,   -15,    -12,    -9,},
    },

    /** For Nsts = 8 */
    {
        {-42,   -39,    -36,    -33,        -39,   -36,    -33,    -30,         -42,   -39,    -36,    -33,         -42,   -39,    -33,    -30,},
        {-39,   -36,    -33,    -30,        -36,   -33,    -30,    -27,         -39,   -36,    -33,    -30,         -36,   -33,    -30,    -27,},
        {-36,   -33,    -30,    -27,        -33,   -30,    -27,    -24,         -36,   -33,    -30,    -27,         -33,   -30,    -27,    -24,},
        {-33,   -30,    -27,    -24,        -30,   -27,    -24,    -21,         -33,   -30,    -27,    -24,         -30,   -27,    -24,    -21,},
        {-30,   -27,    -24,    -21,        -27,   -24     -21,    -18,         -30,   -27,    -24,    -21,         -27,   -24     -21,    -18,},
        {-27,   -24,    -21,    -18,        -24,   -21,    -18,    -15,         -27,   -24,    -21,    -18,         -24,   -21,    -18,    -15,},
        {-24,   -21,    -18,    -15,        -21,   -18,    -15,    -12,         -24,   -21,    -18,    -15,         -21,   -18,    -15,    -12,},
        {-21,   -18,    -15,    -12,        -18,   -15,    -12,    -9,          -21,   -18,    -15,    -12,         -18,   -15,    -12,    -9,},
        {-18,   -15,    -12,    -9,         -15,   -12,    -9,     -6,          -18,   -15,    -12,    -9,         -15,   -12,    -9,     -6,},
        {-15,   -12,    -9,     -6,         -12,   -9,     -6,     -3,          -15,   -12,    -9,     -6,         -12,   -9,     -6,     -3,},
    }
};


/**
 * Get rate_index as per the L_datarate.
 * TODO: Here all rates CCK and OFDM are considered to be registered.
 * So, it needs to be modified as per the rate registered.
 */
static uu_uint8 get_rate_index_of_L_datarate(uu_uint8 L_datarate)
{
    uu_uint8 rate_idx;
    if (L_datarate == UU_HW_RATE_1M)
    {
        rate_idx = 0;
    }
    else if (L_datarate == UU_HW_RATE_2M)
    {
        rate_idx = 1;
    }
    else if (L_datarate == UU_HW_RATE_5M5)
    {
        rate_idx = 2;
    }
    else if (L_datarate == UU_HW_RATE_11M)
    {
        rate_idx = 3;
    }
    else if (L_datarate == UU_HW_RATE_6M)
    {
        rate_idx = 4;
    }
    else if (L_datarate == UU_HW_RATE_9M)
    {
        rate_idx = 5;
    }
    else if (L_datarate == UU_HW_RATE_12M)
    {
        rate_idx = 6;
    }
    else if (L_datarate == UU_HW_RATE_18M)
    {
        rate_idx = 7;
    }
    else if (L_datarate == UU_HW_RATE_24M)
    {
        rate_idx = 8;
    }
    else if (L_datarate == UU_HW_RATE_36M)
    {
        rate_idx = 9;
    }
    else if (L_datarate == UU_HW_RATE_48M)
    {
        rate_idx = 10;
    }
    else if (L_datarate == UU_HW_RATE_54M)
    {
        rate_idx = 11;
    }
    else
    {
        rate_idx = 0;
    }

    return rate_idx;
} /* get_rate_index_of_L_datarate */


uu_int8 uu_wlan_recv_min_sensitivity_table_legacy [UU_SUPPORTED_CCK_RATE + UU_SUPPORTED_OFDM_RATE] =
{
    /** For CCK */
    -79, -78, -77, -76,
    /** For OFDM */
    -82, -81, -79, -77, -74, -70, -66, -65,
};


/** Receiver Minimum Sensitivity depends on MCS index, Bandwidth,
 *  Nsts (STBC), GI, FEC Coding (BCC, LDPC), PHY_TYPE.
 * So, table needs to be maintained as per the above
 * variables to return corresponding sensitivity.
 *
 * Refer to P802.11 ac D3.0 Table 22-25, Pg. 273, it returns the value,
 * But for future we need to extend the table with the above dependencies
 */
uu_int8 uu_get_rec_min_sensitivity (uu_wlan_rx_frame_info_t *rx_fi)
{
    uu_int8 sensitivity;

    /** For VHT */
    uu_uint8 nsts;

    if (rx_fi->rxvec.format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        nsts = rx_fi->rxvec.num_sts;

        if (rx_fi->rxvec.ch_bndwdth == CBW40)
        {
            if (rx_fi->rxvec.is_short_GI)
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_40_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_40_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_40_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_40_bcc_lgi;
                }
            }
        }
        else if (rx_fi->rxvec.ch_bndwdth == CBW80)
        {
            if (rx_fi->rxvec.is_short_GI)
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_80_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_80_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_80_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_80_bcc_lgi;
                }
            }
        }
        else if (rx_fi->rxvec.ch_bndwdth >= CBW160)
        {
            if (rx_fi->rxvec.is_short_GI)
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_160_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_160_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_160_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_160_bcc_lgi;
                }
            }
        }
        else
        {
            if (rx_fi->rxvec.is_short_GI)
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_20_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_20_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_20_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs].mcs_20_bcc_lgi;
                }
            }
        }
    }
    /** For HT */
    else if (rx_fi->rxvec.format == UU_WLAN_FRAME_FORMAT_HT_MF || rx_fi->rxvec.format == UU_WLAN_FRAME_FORMAT_HT_GF)
    {
        nsts = ((rx_fi->rxvec.mcs)/UU_MAX_MCS_PER_STREAM_HT) + rx_fi->rxvec.stbc;

        if (rx_fi->rxvec.ch_bndwdth == CBW40)
        {
            if (rx_fi->rxvec.is_short_GI)
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs & 0x07].mcs_20_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs & 0x07].mcs_20_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs & 0x07].mcs_20_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs & 0x07].mcs_20_bcc_lgi;
                }
            }
        }
        else
        {
            if (rx_fi->rxvec.is_short_GI)
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs & 0x07].mcs_20_ldpc_sgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs & 0x07].mcs_20_bcc_sgi;
                }
            }
            else
            {
                if (rx_fi->rxvec.is_fec_ldpc_coding)
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs & 0x07].mcs_20_ldpc_lgi;
                }
                else
                {
                    sensitivity = uu_wlan_recv_min_sensitivity_table_mcs_g[nsts][rx_fi->rxvec.mcs & 0x07].mcs_20_bcc_lgi;
                }
            }
        }
    }
    else    /** For legacy */
    {
        uu_uint8 rate_index;
        rate_index = get_rate_index_of_L_datarate(rx_fi->rxvec.L_datarate);

#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("rate_index : %d\n", rate_index);
#endif
        sensitivity = uu_wlan_recv_min_sensitivity_table_legacy[rate_index];
    }

    return sensitivity;
} /* uu_get_rec_min_sensitivity  */


/** This function is to send TPC Report */
uu_void uu_wlan_process_and_send_tpc_report (uu_mgmt_action_frame_t *tpc_request, uu_wlan_rx_frame_info_t *rx_fi)
{
    struct sk_buff *skb;
    uu_mgmt_action_frame_t *tpc_report;

    struct ieee80211_tx_info *info;
    int i;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Preparing TPC_REPORT frame \n");
#endif

    skb = dev_alloc_skb(sizeof(uu_mgmt_action_frame_t));
    if (!skb)
        return;

    /** 24 is the number of octets of management frame format
      excluding HT_Control, Frame_body and FCS */
    tpc_report = (uu_mgmt_action_frame_t *)skb_put(skb, 24);
    /** Need to fill: frame_control, duration, da, sa, bssid,
     * seq_ctrl, tpc_request_frame (action, category, dialog_token,
     * tpc_request_element (element id, length)) */
    memset(tpc_report, 0, 24);
    memcpy(tpc_report->sa, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
    memcpy(tpc_report->da, tpc_request->sa, ETH_ALEN);
    memcpy(tpc_report->bssid, tpc_request->bssid, ETH_ALEN);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Source Address: %pM\nDestination Address: %pM\nBSSID: %pM\n",
    tpc_report->sa, tpc_report->da, tpc_report->bssid);
#endif

    tpc_report->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);

    /** Spectrum Management TPC Request frame
     * Category : 1 octet, Value is 0.
     * Spectrum Management Action : 1 octet, Value is 3.
     * Dialog Token : 1 octet, Value is non-zero and contains the ID of request/report transaction.
     * TPC Report element : 2 octets.
     *
     * TPC Report element:-
     * Element ID : 1 octet, Value is 35.
     * Length : 1 octet, Value is 0.
     * Transmit Power : 1 octet, Value will be filled by B_TPL.
     * Link Margin : 1 octet, Value is RSSI - Receiver Minimum Sensitivity.
     */

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Preparing TPC_REPORT frame\n");
#endif
    /** cate (1), mgmt_axn(1), dialog_token(1), e_id(1), length (1), size of tpc report */
    skb_put(skb, 3 + sizeof(uu_wlan_tpc_rep_element_t));
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len 2: %d\n", skb->len);
#endif

    tpc_report->uu_meas_frame.category = WLAN_CATEGORY_SPECTRUM_MGMT;
    tpc_report->uu_meas_frame.mgmt_action = WLAN_ACTION_SPCT_TPC_RPRT;
    tpc_report->uu_meas_frame.dialog_token = tpc_request->uu_meas_frame.dialog_token;

    memset(&tpc_report->uu_meas_frame.u.tpc_rep_element, 0, sizeof(uu_wlan_tpc_rep_element_t));
    tpc_report->uu_meas_frame.u.tpc_rep_element.length = 2;
    tpc_report->uu_meas_frame.u.tpc_rep_element.element_id = WLAN_EID_TPC_REPORT;
    /** TODO: Need to take care of link margin calculation,
      as per the resolution of rssi and sensitivity */
    tpc_report->uu_meas_frame.u.tpc_rep_element.link_margin
                        = uu_wlan_rssi_level_to_rssi_power[rx_fi->rxvec.rssi] - uu_get_rec_min_sensitivity(rx_fi);
    tpc_report->uu_meas_frame.u.tpc_rep_element.transmit_power = 0; /* This will be modified in umac/src/uu_mac_tx.c */

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len:%d: ", skb->len);
    for (i = 0; i < skb->len; i++)
    {
        printk("%x ", skb->data[i]);
    }
    printk("\n ");
#endif

    skb_set_queue_mapping(skb, IEEE80211_AC_VO);
    skb->priority = 7;

    info = IEEE80211_SKB_CB(skb);
    info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;

    __send_frame(skb);
} /* uu_wlan_process_and_send_tpc_report */


/** This function is to send TPC Request */
uu_void uu_wlan_prepare_and_send_tpc_request (uu_void)
{
    struct sk_buff *skb;
    uu_mgmt_action_frame_t *tpc_request;
    struct ieee80211_tx_info *info;
    int i;
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Preparing TPC_REQUEST frame \n");
#endif

    skb = dev_alloc_skb(sizeof(uu_mgmt_action_frame_t));
    if (!skb)
        return;

    /** 24 is the number of octets of management frame format
      excluding HT_Control, Frame_body and FCS */
    tpc_request = (uu_mgmt_action_frame_t *)skb_put(skb, 24);
    /** Need to fill: frame_control, duration, da, sa, bssid,
     * seq_ctrl, tpc_request_frame (action, category, dialog_token,
     * tpc_request_element (element id, length)) */
    memset(tpc_request, 0, 24);
    memcpy(tpc_request->da, uu_wlan_context_gp->assoc_sta, ETH_ALEN);
    memcpy(tpc_request->sa, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
    memcpy(tpc_request->bssid, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Source Address: %pM\nDestination Address: %pM\nBSSID: %pM\n",
    tpc_request->sa, tpc_request->da, tpc_request->bssid);
#endif

    tpc_request->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);

    /** Spectrum Management TPC Request frame
     * Category : 1 octet, Value is 0.
     * Spectrum Management Action : 1 octet, Value is 2.
     * Dialog Token : 1 octet, Value is non-zero and contains the ID of request/report transaction.
     * TPC Request element : 2 octets.
     *
     * TPC Request element:-
     * Element ID : 1 octet, Value is 34.
     * Length : 1 octet, Value is 0.
     */

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Preparing TPC_REQUEST frame \n");
#endif
    /** cate (1), mgmt_axn(1), dialog_token(1), tpc_request(e_id(1), length (1)) */
    skb_put(skb, 3 + sizeof(uu_wlan_tpc_req_element_t));
    tpc_request->uu_meas_frame.category = WLAN_CATEGORY_SPECTRUM_MGMT;
    /** Presently only for spectrum management tpc request */
    tpc_request->uu_meas_frame.mgmt_action = WLAN_ACTION_SPCT_TPC_REQ;
    tpc_request->uu_meas_frame.dialog_token = 89;

    memset(&tpc_request->uu_meas_frame.u.tpc_req_element, 0, sizeof(uu_wlan_tpc_req_element_t));
    tpc_request->uu_meas_frame.u.tpc_req_element.element_id = WLAN_EID_TPC_REQUEST;
    tpc_request->uu_meas_frame.u.tpc_req_element.length = 0;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len:%d: ", skb->len);
    for (i = 0; i < skb->len; i++)
    {
    printk("%x ", skb->data[i]);
    }
    printk("\n ");
#endif

    skb_set_queue_mapping(skb, IEEE80211_AC_VO);
    skb->priority = 7;

    info = IEEE80211_SKB_CB(skb);
    info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;

    __send_frame(skb);
} /* uu_wlan_prepare_and_send_tpc_request */


/**-------------------------------------------------------------------------------------
 * ----------------------------HANDLING TPC TIMER---------------------------------------
 * -------------------------------------------------------------------------------------
 */

#define MicroS_TO_NS(x) (x * 1000)

static struct hrtimer uu_wlan_tpc_hr_timer_g;
static uu_int8   exit_flag_g = 0;
#ifdef UU_TPC_HRTIMER_CONST
static uu_uint32 hrtimer_count = 0;
#endif
enum hrtimer_restart uu_wlan_tpc_hrtimer_cbk (struct hrtimer *timer)
{
    ktime_t timeout, cur_time;
    unsigned long misses;
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("TPC Request Timer Handler \n");
#endif
#ifdef UU_TPC_HRTIMER_CONST
    if (hrtimer_count == 10)
    {
        return HRTIMER_NORESTART;
    }
#endif
    if (exit_flag_g == 1)
    {
        return HRTIMER_NORESTART;
    }
    uu_wlan_prepare_and_send_tpc_request();
#ifdef UU_TPC_HRTIMER_CONST
    hrtimer_count ++;
#endif
    timeout = ktime_set(0, MicroS_TO_NS(TPC_Request_Timer_Interval));
    cur_time = ktime_get();

    misses = hrtimer_forward(timer, cur_time, timeout);
    return HRTIMER_RESTART;
} /* uu_wlan_tpc_hrtimer_cbk */

uu_int32 uu_wlan_tpc_timer_start(uu_void)
{
    ktime_t ktime;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("TPC Timer starting \n");

    UU_WLAN_LOG_DEBUG(("LMAC: TPC Request HR Timer module starting\n"));
#endif

    exit_flag_g = 0;
    ktime = ktime_set(0, MicroS_TO_NS(TPC_Request_Timer_Interval));

    hrtimer_init(&uu_wlan_tpc_hr_timer_g, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    uu_wlan_tpc_hr_timer_g.function = &uu_wlan_tpc_hrtimer_cbk;

#ifdef UU_WLAN_DFSTPC_DEBUG
    UU_WLAN_LOG_INFO(("Starting TPC Req timer to fire in %dus (%ld)\n", TPC_Request_Timer_Interval, jiffies));
#endif

    hrtimer_start( &uu_wlan_tpc_hr_timer_g, ktime, HRTIMER_MODE_REL );

    return UU_SUCCESS;
} /* uu_wlan_tpc_timer_start */

uu_int32 uu_wlan_tpc_timer_stop (uu_void)
{
    int ret;

    exit_flag_g = 1;
    ret = hrtimer_cancel(&uu_wlan_tpc_hr_timer_g);

#ifdef UU_WLAN_DFSTPC_DEBUG
    if (ret)
    {
        printk("The TPC Request timer was still in use...\n");
    }
    printk("TPC Request HR Timer module uninstalling\n");
#endif

    return UU_SUCCESS;
}


/**-------------------------------------------------------------------------------------
 * ----------------------------SPECIFIC FUNCTIONS---------------------------------------
 * ----------------------------MAPPING OFDM AND 11AC TABLE------------------------------
 * -------------------------------------------------------------------------------------
 */

/**
 * Mapping of OFDM TPC Table and VHT TPC Table
 *
 * l2 < l1 < n2 < n1 < m2 < m1 < h2 < h1
 *
 *          OFDM TPL    VHT TPL
 *  l2[0]   Level1      Level9
 *  l1[0]   Level2      Level24
 *  n2[0]   Level3      Level39
 *  n1[0]   Level4      Level54
 *  m2[0]   Level5      Level69
 *  m1[0]   Level6      Level84
 *  h2[0]   Level7      Level99
 *  h1[0]   Level8      Level114
 *
 */

uu_uint8 get_ofdm_tpl_from_vht_tpl (uu_uint8 vht_tpl)
{
    uu_uint8 ofdm_tpl;

    if(uu_wlan_tp_level_diff_g)
    {
        ofdm_tpl = ((vht_tpl - UU_WLAN_TPLevel9) / uu_wlan_tp_level_diff_g) + 1;
    }
    else
    {
        ofdm_tpl = UU_WLAN_TPLevel8;
    }
    /**
     * TODO:
     * NOTE: The above equation is valid only if the TPL for OFDM and VHT
     * are arranged taking care of the transmit power for each base levels
     * (h1, h2, m1, m2, .....)
     * else, the above line needs to be replaced by the below line
     * ofdm_tpl = uu_wlan_dBm_to_TPLevel (uu_wlan_transmit_power_dBm [vht_tpl],
                                    UU_WLAN_TPLevel1, dot11NumberSupportedPowerLevelsImplemented_g);
     */

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("VHT TPL: %d, OFDM TPL: %d\n", vht_tpl, ofdm_tpl);
#endif
    return ofdm_tpl;
} /* get_ofdm_tpl_from_vht_tpl */


/**
 * Mapping of OFDM TPC Table and VHT TPC Table
 *
 * CCK TPL will use the same level as OFDM
 *
 *          OFDM TPL    CCK TPL
 * l2           1           2
 * l1           2           2
 * n2           3           4
 * n1           4           4
 * m2           5           6
 * m1           6           6
 * h2           7           8
 * h1           8           8
 *
 */
uu_uint8 get_cck_tpl_from_ofdm_tpl (uu_uint8 ofdm_tpl)
{
    return (ofdm_tpl  + (ofdm_tpl % 2));
}


/**-------------------------------------------------------------------------------------
 * ----------------------------SPECIFIC FUNCTIONS---------------------------------------
 * ----------------------------UPDATING B_TPL (INC/DEC) AS PER INPUT--------------------
 * -------------------------------------------------------------------------------------
 */

#ifdef UU_WLAN_DFSTPC_DEBUG
/** This is only for debugging purpose */
static uu_void print_btpl (uu_void)
{
    int i, j;
    printk("BK\tBE\tVI\tVO\n");
    for (i = 0; i < 4; i ++)
    {
        for (j = 0; j < 4; j ++)
        {
            printk("%d\t", uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + j]);
        }
        printk("\n");
    }
}
#endif

/** This function is for increasing the B_TPL as per the input level, bandwidth, ac */
static uu_void uu_wlan_inc_BTPL_Level (uu_uint8 inc_level, uu_uint8 bandwidth, uu_uint8 ac)
{
    uu_uint8 max_level;

    /** First identify the maximum transmit power level */
    if (uu_wlan_btpl_ac_g[0] < UU_WLAN_TPLevel9)
    {
        max_level = uu_wlan_utpl_threshold_11n_g[bandwidth];
    }
    else
    {
        max_level = uu_wlan_utpl_threshold_11ac_g[bandwidth];
    }

    uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] += inc_level;
    if (uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] > max_level)
    {
        uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] = max_level;
    }
} /* uu_wlan_inc_BTPL_Level */


/** This function is for increasing the B_TPL as per the input level */
static uu_void uu_wlan_inc_BTPL (uu_uint8 inc_level)
{
    uu_uint8 i, j;

    /** Increase the B_TPL maximum to maximum transmit power level */
    for (i = 0; i <= TPC_Max_Bandwidth; i++)
    {
        for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
        {
            uu_wlan_inc_BTPL_Level (inc_level, i, j);
        }
    }
}


/** This function is for increasing the B_TPL as per the input new tpc in dBm, bandwidth, ac */
static uu_void uu_wlan_inc_BTPL_dBm (uu_uint8 tpc_dBm, uu_uint8 bandwidth, uu_uint8 ac)
{
    uu_uint8 max_level;

    /** First identify the maximum transmit power level */
    if (uu_wlan_btpl_ac_g[0] < UU_WLAN_TPLevel9)
    {
        max_level= uu_wlan_utpl_threshold_11n_g[bandwidth];
    }
    else
    {
        max_level = uu_wlan_utpl_threshold_11ac_g[bandwidth];
    }

    if (uu_wlan_transmit_power_dBm[max_level] < tpc_dBm)
    {
        uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] = max_level;
    }
    else
    {
        while (uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]] < tpc_dBm)
        {
            uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]++;
        }
        if (uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]] > tpc_dBm)
        {
            uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]--;
        }
    }
} /* uu_wlan_inc_BTPL_dBm */


/** This function is for decreasing the B_TPL as per the input level, bandwidth, ac */
static uu_void uu_wlan_dec_BTPL_Level (uu_uint8 dec_level, uu_uint8 bandwidth, uu_uint8 ac)
{
    if ((uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] > dec_level))
    {
        uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] -= dec_level;
        if (uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] < uu_wlan_lowpload_datamgmt_ltpl[(bandwidth * UU_WLAN_AC_MAX_AC) + ac])
        {
            uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] = uu_wlan_lowpload_datamgmt_ltpl[(bandwidth * UU_WLAN_AC_MAX_AC) + ac];
        }
    }
    else
    {
        uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] = uu_wlan_lowpload_datamgmt_ltpl[(bandwidth * UU_WLAN_AC_MAX_AC) + ac];
    }
}

/** This function is for decreasing the B_TPL as per input level */
static uu_void uu_wlan_dec_BTPL (uu_uint8 dec_level)
{
    uu_uint8 i, j;

    /* Decrease the B_TPL minimum to lower transmit power level */
    for (i = 0; i <= TPC_Max_Bandwidth; i++)
    {
        for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
        {
            uu_wlan_dec_BTPL_Level (dec_level, i, j);
        }
    }
}

/** This function is for decreasing the B_TPL as per the input new tpc in dBm, bandwidth and ac */
static uu_void uu_wlan_dec_BTPL_dBm (uu_uint8 tpc_dBm, uu_uint8 bandwidth, uu_uint8 ac)
{
    if (uu_wlan_transmit_power_dBm[uu_wlan_lowpload_datamgmt_ltpl[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]] >= tpc_dBm)
    {
        uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] = uu_wlan_lowpload_datamgmt_ltpl[(bandwidth * UU_WLAN_AC_MAX_AC) + ac];
    }
    else
    {
        while (uu_wlan_transmit_power_dBm[(uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac])] >= tpc_dBm)
        {
            if ((uu_wlan_transmit_power_dBm[(uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]) - 1] < tpc_dBm)
                && (uu_wlan_transmit_power_dBm[(uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac])] == tpc_dBm))
            {
                break;
            }
            uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]--;
        }
    }
}


/**-------------------------------------------------------------------------------------
 * ----------------------------SPECIFIC FUNCTIONS---------------------------------------
 * ----------------------------UPDATING B_TPL AS PER TPC REPORT-------------------------
 * -------------------------------------------------------------------------------------
 */

/** Increasing B_TPL as per the link margin */
static uu_void uu_lm_inc_btpl_value_all(uu_void)
{
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Printing uu_wlan_btpl_ac_g before increasing as per the link margin\n");
    print_btpl();
#endif
    /** First identify the maximum transmit power level */
    if (uu_wlan_btpl_ac_g[0] < UU_WLAN_TPLevel9)
    {
        TPC_Link_Min_Inc = TPC_Link_Min_Ofdm_Inc;
    }
    else
    {
        TPC_Link_Min_Inc = TPC_Link_Min_11ac_Inc;
    }

    /* Increase B_TPL as per the link margin increment */
    uu_wlan_inc_BTPL (TPC_Link_Min_Inc);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Printing uu_wlan_btpl_ac_g after increasing as per the link margin\n");
    print_btpl();
#endif
}


/** Decreasing B_TPL as per the link margin */
static uu_void uu_lm_dec_btpl_value_all(uu_void)
{
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Printing uu_wlan_btpl_ac_g before decreasing as per the link margin\n");
    print_btpl();
#endif

    /** First identify the maximum transmit power level,
     and accordingly assign the value for decrementing. */
    if (uu_wlan_btpl_ac_g[0] < UU_WLAN_TPLevel9)
    {
        TPC_Link_Max_Dec = TPC_Link_Max_Ofdm_Dec;
    }
    else
    {
        TPC_Link_Max_Dec = TPC_Link_Max_11ac_Dec;
    }

    /** Decrease B_TPL as per the link margin decrement */
    uu_wlan_dec_BTPL (TPC_Link_Max_Dec);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Printing uu_wlan_btpl_ac_g after decreasing as per the link margin\n");
    print_btpl();
#endif
}


/**
 * When the path loss changes from the previous, the TPC changes (increases/decreases)
 * by ((new_path_loss – previous_path_loss) * 3) dBm for each AC/bandwidth.
 * Multiplying by 3 indicates that either the TPC is getting half or getting
 * doubled in units of mW. The new TPC should not exceed U_TPL_Threshold or come below L_TPL.
 * For 11n case, we increase TPC by 1 level or decrease TPC by 1 level for each  AC/bandwidth.
 */
static uu_void uu_pl_update_btpl_value_all(int path_loss_range, int new_path_range)
{
    uu_int8 delta;
    uu_uint8 i, j, new_tpc_dBm_value;
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Printing uu_wlan_btpl_ac_g before updating as per the path loss\n");
    print_btpl();
#endif
    delta = new_path_range - path_loss_range;
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("delta (new_path_range - path_loss_range) is: %d\n", delta);
#endif

    /** Need to decrement TPC */
    if (delta < 0)
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Since delta is negative, we will decrease the B_TPL\n");
#endif
        for (i = 0; i <= TPC_Max_Bandwidth; i++)
        {
            for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
            {
                /** Decrease till the dBm decreases by 3, making it half */
                if (uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + j]] > UU_WLAN_DBM_DOUBLING_FACTOR)
                {
                    new_tpc_dBm_value = uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + j]]
                                                            - UU_WLAN_DBM_DOUBLING_FACTOR;
                    uu_wlan_dec_BTPL_dBm (new_tpc_dBm_value, i, j);
                }
            }
        }
    }
    /** Need to increment TPC */
    else
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Since delta is positive, we will increase the B_TPL\n");
#endif
        for (i = 0; i <= TPC_Max_Bandwidth; i++)
        {
            for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
            {
                /** Increase till the dBm increases by 3, making it double */
                new_tpc_dBm_value = uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + j]]
                                                    + UU_WLAN_DBM_DOUBLING_FACTOR;
                uu_wlan_inc_BTPL_dBm (new_tpc_dBm_value, i, j);
            }
        }
    }
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Printing uu_wlan_btpl_ac_g after updating as per the path loss\n");
    print_btpl();
#endif
} /* uu_pl_update_btpl_value_all */


/**
 * LINK MARGIN
 *
 * Link Margin = RSSI - Minimum Receiver Sensitivity
 *          where minimum receiver sensitivity depends on modulation and bandwidth.
 *
 * If Link Margin <= Min_Link_Threshold, then increase the TPC by TPC_Link_Min_Inc (5) sub-levels for each AC and bandwidth.
 * If Link Margin >= Max_Link_Threshold, then decrease the TPC by TPC_Link_Max_Dec (3) sub-levels for each AC and bandwidth.
 *
 * For 11n case, we increase TPC by 1 level or decrease TPC by 1 level for each AC/bandwidth.
 *
 * PATH LOSS
 *
 * Path Loss (dB) = TxPower (present in TPC Report) - RSSI (AP received TPC Report)
 *
 * Using path loss, we can either determine the distance between the transmitter or receiver,
 * (either true if no interference, or the relative distance as per the interference)
 *
 * Distance as per the path loss in metres, d = 10^(PL/20)    * (c/(4 * pi * fc))
 *          where,
 *                PL = Path Loss in dB
 *                c = 3 * 10^8 m/s
 *                fc = carrier frequency in Hz.
 * Thus, relative to the distance, we divide the path loss (PL) level.
 *
 * If distance < 30 m, d_category = 1, PL < 72 dB
 * 20 m <= distance < 50 m, d_category = 2, 72 dB <= PL < 80 dB
 * 50 m <= distance < 100 m, d_category = 3, 80 dB <= PL < 86 dB
 * 100 m < distance, d_category = 4, PL >= 86 dB
 *
 * Since, distance are almost getting doubled, so, we double the TPC as per mW or we increase TPC with 3 dBm
 * for each AC and bandwidth, considering the distance shown by the path loss in previous received TPC Report.
 *
 * So, when the d_category changes from the previous, the TPC changes (increases/decreases)
 * by ((d_category_new - d_category_previous) * 3) dBm. The new TPC should not exceed upper or come below lower limit of TPC.
 *
 * For 11n case, we increase TPC by 1 level or decrease TPC by 1 level for each AC/bandwidth.
 *
 * Note: If both Link Margin and Path loss leads to the change in TPC level, then we consider Path loss only.
 * The decision as per link margin will be discarded.
 */
uu_void uu_wlan_process_tpc_report (uu_mgmt_action_frame_t *tpc_report, uu_uint8 rssi)
{
    uu_int16 new_path_loss_value, new_path_range;
#ifdef UU_WLAN_DFSTPC_DEBUG
    /** Calculate path loss in dB */
    printk("tpc_report->uu_meas_frame.u.tpc_rep_element.transmit_power : %d\n", tpc_report->uu_meas_frame.u.tpc_rep_element.transmit_power);
    printk("uu_wlan_rssi_level_to_rssi_power[rssi] : %d\n", uu_wlan_rssi_level_to_rssi_power[rssi]);
#endif
    new_path_loss_value = (tpc_report->uu_meas_frame.u.tpc_rep_element.transmit_power) - uu_wlan_rssi_level_to_rssi_power[rssi];
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("new_path_loss_value = %d, path_loss_value = %d\n", new_path_loss_value, path_loss_value);
#endif

    if (path_loss_value < 0)
    {
        path_loss_value = new_path_loss_value;
    }

    /* TODO: Instead we can try to give more weightage to the max (path_loss_value, new_path_loss_value) */
    path_loss_value = (path_loss_value * (100 - PATH_LOSS_EWMA_LEVEL) + new_path_loss_value * PATH_LOSS_EWMA_LEVEL) / 100;
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Cumulative Path Loss Value: %d\n", path_loss_value);
#endif

    /** Determine the range in dBm and assign to new_path_range */
    if (path_loss_value < Path_Loss_MaxValue_LessThan_20m)
    {
        new_path_range = Path_Loss_Range_LessThan_20m;
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("New Path Loss is in the range less than 20 metres\n");
#endif
    }
    else
    {
        if (path_loss_value < Path_Loss_MaxValue_LessThan_50m)
        {
            new_path_range = Path_Loss_Range_LessThan_50m;
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("New Path Loss is in the range less than 50 metres\n");
#endif
        }
        else
        {
            if (path_loss_value < Path_Loss_MaxValue_LessThan_100m)
            {
                new_path_range = Path_Loss_Range_LessThan_100m;
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("New Path Loss is in the range less than 100 metres\n");
#endif
            }
            else
            {
                new_path_range = Path_Loss_Range_Beyond_100m;
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("New Path Loss is in the range more than 100 metres\n");
#endif
            }
        }
    }

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Path Loss Range: %d, new_path_range: %d\n", path_loss_range, new_path_range);
#endif
    /** If its the first TPC Report received, then no need to updating the B_TPL */
    if (path_loss_range < 0)
    {
        path_loss_range = new_path_range;
    }

    else/** Priority will be given to Path Loss for updating B_TPL */
    {
        if (path_loss_range == new_path_range)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Link margin is: %d, Minimum Link Threshold: %d, Maximum Link Threshold: %d\n",
                    tpc_report->uu_meas_frame.u.tpc_rep_element.link_margin,
                    Min_Link_Threshold, Max_Link_Threshold);
#endif
            /** Link Margin will be considered for updating B_TPL */
            if (tpc_report->uu_meas_frame.u.tpc_rep_element.link_margin <= Min_Link_Threshold)
            {
                uu_lm_inc_btpl_value_all();
#ifdef UU_WLAN_DFSTPC_DEBUG
                printk("Link margin in the TPC Report is less than Minimum Link Threshold\n");
#endif
            }

            if (tpc_report->uu_meas_frame.u.tpc_rep_element.link_margin > Max_Link_Threshold)
            {
                uu_lm_dec_btpl_value_all();
#ifdef UU_WLAN_DFSTPC_DEBUG
                printk("Link margin in the TPC Report is greater than Maximum Link Threshold\n");
#endif
            }
        }
        else
        {
            /** Path Loss will be considered for updating B_TPL */

            /** Update the B_TPL as per the difference in the new path range and previous one */
            uu_pl_update_btpl_value_all(path_loss_range, new_path_range);

            /** Update path_loss_range */
            path_loss_range = new_path_range;
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Path Loss is considered to update B_TPL\n");
#endif
        }
    }
} /* uu_wlan_process_tpc_report */


/**-------------------------------------------------------------------------------------
 * ----------------------------SPECIFIC FUNCTIONS---------------------------------------
 * ----------------------------UPDATING B_TPL AND TPC AS PER NEW MCS AND N_TX-----------
 * -------------------------------------------------------------------------------------
 */

uu_uint8 uu_increase_tpl_new_htmcs (uu_uint8 ht_mcs, int normal_htmcs, uu_uint8 bdwdth, uu_uint8 tpl)
{
    uu_uint8 new_tpl = tpl;
    uu_int8 delta_ntx = 0;
    uu_int8 delta_mcs = 0;
    uu_uint8 max_level;
    uu_int32 new_tpc_dBm_value;

    /** These calculations are valid only for HT with mcs index < 32 */
    delta_ntx = ((ht_mcs >> 3) & 0x03) - ((normal_htmcs >> 3) & 0x03);
    delta_mcs = (ht_mcs & UU_HT_MANDATORY_MCS_MASK) - (normal_htmcs & UU_HT_MANDATORY_MCS_MASK);

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Delta_ntx: %d, Delta_mcs: %d\n", delta_ntx, delta_mcs);
#endif
    /** For 2.4 GHz, increment only considering Nss if both nss and/or mcs is increased,
     * else increase 1 level if only mcs is increased */
    if (tpl < UU_WLAN_TPLevel9)
    {
        /** First identify the maximum transmit power level */
        max_level = uu_wlan_utpl_threshold_11n_g[0];

        /** If ntx increases, then we won't consider change in mcs index */
        if (delta_ntx > 0)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL before changing at 5 GHz\n");
            printk("TPL: %d\n", tpl);
#endif
            new_tpc_dBm_value = uu_wlan_transmit_power_dBm[tpl]
                                    + (UU_WLAN_DBM_DOUBLING_FACTOR * CEIL(delta_ntx, (((normal_htmcs >> 3) & 0x03) + 1)));
            if (uu_wlan_transmit_power_dBm[max_level] < new_tpc_dBm_value)
            {
                new_tpl = max_level;
            }
            else
            {
                while (uu_wlan_transmit_power_dBm[new_tpl] < new_tpc_dBm_value)
                {
                    new_tpl ++;
                }
                if (uu_wlan_transmit_power_dBm[new_tpl] > new_tpc_dBm_value)
                {
                    new_tpl --;
                }
            }
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL as per delta_ntx at 5 GHz\n");
            printk("NEW TPL: %d\n", new_tpl);
#endif
        }
        else
            /** mcs must have increased */
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL before changing at 5 GHz\n");
            printk("TPL: %d\n", tpl);
#endif
            new_tpl = (((tpl + 1) > max_level) ? max_level : (tpl + 1));
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL as per delta_mcs at 5 GHz\n");
            printk("NEW TPL: %d\n", new_tpl);
#endif
        }
    }

    /** For 5 GHz, when we have lots of levels for base_tpl */
    else
    {
        /** First identify the maximum transmit power level */
        max_level = uu_wlan_utpl_threshold_11ac_g[bdwdth];

        /** Increase in TPC as per increase in mcs index */
        if (delta_mcs > 0)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL before changing at 5 GHz\n");
            printk("TPL: %d\n", tpl);
#endif
            /** Increasing by delta_mcs levels */
            new_tpl = (((tpl + delta_mcs) > max_level) ? max_level : (tpl + delta_mcs));
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL as per delta_mcs at 5 GHz\n");
            printk("NEW TPL: %d\n", new_tpl);
#endif
        }

        /** Increase in TPC as per increase in number of spatial streams */
        if (delta_ntx > 0)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL before changing at 5 GHz\n");
            printk("TPL: %d\n", tpl);
#endif
            /** Double the TP as per the increment in number of transmit streams */
            new_tpc_dBm_value = new_tpl + (UU_WLAN_DBM_DOUBLING_FACTOR * delta_ntx);

            if (uu_wlan_transmit_power_dBm[max_level] < new_tpc_dBm_value)
            {
                new_tpl = max_level;
            }
            else
            {
                while (uu_wlan_transmit_power_dBm[new_tpl] < new_tpc_dBm_value)
                {
                    new_tpl ++;
                }
                if (uu_wlan_transmit_power_dBm[new_tpl] > new_tpc_dBm_value)
                {
                    new_tpl --;
                }
            }
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL as per delta_ntx at 5 GHz\n");
            printk("NEW TPL: %d\n", new_tpl);
#endif
        }
    }
    return new_tpl;
} /* uu_increase_tpl_new_htmcs */


uu_uint8 uu_increase_tpl_new_vhtmcs (uu_uint8 vht_mcs, int normal_vhtmcs, uu_uint8 bdwdth, uu_uint8 tpl)
{
    uu_uint8 new_tpl = tpl;
    uu_int8 delta_ntx = 0;
    uu_int8 delta_mcs = 0;
    uu_uint8 max_level;
    uu_int32 new_tpc_dBm_value;

    delta_ntx = (vht_mcs & UU_VHT_NSS_FROM_MCS_MASK) - (normal_vhtmcs & UU_VHT_NSS_FROM_MCS_MASK);
    delta_mcs = ((vht_mcs >> UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & UU_VHT_MCSINDEX_FROM_MCS_MASK)
                  - ((normal_vhtmcs >> UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & UU_VHT_MCSINDEX_FROM_MCS_MASK);

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Delta_ntx: %d, Delta_mcs: %d\n", delta_ntx, delta_mcs);
#endif
    /** First identify the maximum transmit power level */
    max_level = uu_wlan_utpl_threshold_11ac_g[bdwdth];

    /** Increase in TPC as per increase in mcs index */
    if (delta_mcs > 0)
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL before changing at 5 GHz\n");
            printk("TPL: %d\n", tpl);
#endif
        /** Increasing by delta_mcs levels */
        new_tpl = (((tpl + delta_mcs) > max_level) ? max_level : (tpl + delta_mcs));
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary VHT TPL as per delta_mcs at 5 GHz\n");
            printk("NEW TPL: %d\n", new_tpl);
#endif
    }

    /** Increase in TPC as per increase in number of spatial streams */
    if (delta_ntx > 0)
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary HT TPL before changing at 5 GHz\n");
            printk("TPL: %d\n", tpl);
#endif
        /** Double the TP as per the increment in number of transmit streams */
        new_tpc_dBm_value = uu_wlan_transmit_power_dBm[new_tpl]
                                    + (UU_WLAN_DBM_DOUBLING_FACTOR * CEIL(delta_ntx, ((normal_vhtmcs & 0x07) + 1)));

        if (uu_wlan_transmit_power_dBm[max_level] < new_tpc_dBm_value)
        {
            new_tpl = max_level;
        }
        else
        {
            while (uu_wlan_transmit_power_dBm[new_tpl] < new_tpc_dBm_value)
            {
                new_tpl ++;
            }
            if (uu_wlan_transmit_power_dBm[new_tpl] > new_tpc_dBm_value)
            {
                new_tpl --;
            }
        }
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing temporary VHT TPL as per delta_ntx at 5 GHz\n");
            printk("NEW TPL: %d\n", new_tpl);
#endif
    }
    return new_tpl;
} /* uu_increase_tpl_new_vhtmcs */


uu_void uu_update_btpl_new_htmcs_all(uu_uint8 ht_mcs, int normal_htmcs)
{
    uu_int8 delta_ntx = 0;
    uu_int8 delta_mcs = 0;
    uu_int8 new_tpc_dBm_value;
    uu_uint8 i,j;

    /** These calculations are valid only for HT with mcs index < 32 */
    delta_ntx = ((ht_mcs >> 3) & 0x03) - ((normal_htmcs >> 3) & 0x03);
    delta_mcs = (ht_mcs & UU_HT_MANDATORY_MCS_MASK) - (normal_htmcs & UU_HT_MANDATORY_MCS_MASK);

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Delta_ntx: %d, Delta_mcs: %d\n", delta_ntx, delta_mcs);
#endif
    /** For 2.4 GHz, increment only considering Nss if both nss and/or mcs is increased,
     * else increase 1 level if only mcs is increased */
    if (uu_wlan_btpl_ac_g[0] < UU_WLAN_TPLevel9)
    {
        /** If ntx increases, then we won't consider change in mcs index */
        if (delta_ntx > 0)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing uu_wlan_btpl_ac_g before changing as per delta_ntx at 2.4 GHz\n");
            print_btpl();
#endif
            for (i = 0; i <= CBW40; i++)
            {
                for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
                {
                    /** Increase till the dBm increases by 3, making it double */
                    new_tpc_dBm_value = uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + j]]
                                            + (UU_WLAN_DBM_DOUBLING_FACTOR * CEIL(delta_ntx, (((normal_htmcs >> 3) & 0x03) + 1)));

                    uu_wlan_inc_BTPL_dBm (new_tpc_dBm_value, i, j);
                }
            }
        }
        else
        /** mcs must have increased */
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing uu_wlan_btpl_ac_g before changing as per delta_mcs at 2.4 GHz\n");
            print_btpl();
#endif
            for (i = 0; i <= CBW40; i++)
            {
                for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
                {
                    uu_wlan_inc_BTPL_Level (delta_mcs, i, j);
                }
            }
        }
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Printing uu_wlan_btpl_ac_g after changing as per the new ht rate in 2.4 GHz\n");
        print_btpl();
#endif
    }

    /** For 5 GHz, when we have lots of levels for base_tpl.
     * So, first we consider increasing B_TPL as per increase in MCS,
     * then we increase B_TPL as per increase in Nss */
    else
    {
        /** Increase in TPC as per increase in mcs index */
        if (delta_mcs > 0)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Printing uu_wlan_btpl_ac_g before changing as per delta_mcs at 5 GHz\n");
            print_btpl();
#endif
            /** Increasing by delta_mcs levels */
            for (i = 0; i <= TPC_Max_Bandwidth; i++)
            {
                for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
                {
                    uu_wlan_inc_BTPL_Level (delta_mcs, i, j);
                }
            }
        }

        /** Increase in TPC as per increase in number of spatial streams */
        if (delta_ntx > 0)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Printing uu_wlan_btpl_ac_g before changing as per delta_ntx at 5 GHz\n");
        print_btpl();
#endif
            for (i = 0; i <= TPC_Max_Bandwidth; i++)
            {
                for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
                {
                    new_tpc_dBm_value = uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + j]]
                                            + (UU_WLAN_DBM_DOUBLING_FACTOR * CEIL(delta_ntx, (((normal_htmcs >> 3) & 0x03) + 1)));

                    uu_wlan_inc_BTPL_dBm (new_tpc_dBm_value, i, j);
                }
            }
        }
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Printing uu_wlan_btpl_ac_g after changing as per the new ht at 5 GHz\n");
        print_btpl();
#endif
    }
} /* uu_update_btpl_new_htmcs_all */


uu_void uu_update_btpl_new_vhtmcs_all(uu_uint8 vht_mcs, int normal_vhtmcs)
{
    uu_int8 delta_ntx = 0;
    uu_int8 delta_mcs = 0;
    uu_int32 new_tpc_dBm_value;
    int i,j;

    delta_ntx = (vht_mcs & UU_VHT_NSS_FROM_MCS_MASK) - (normal_vhtmcs & UU_VHT_NSS_FROM_MCS_MASK);
    delta_mcs = ((vht_mcs >> UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & UU_VHT_MCSINDEX_FROM_MCS_MASK)
                   - ((normal_vhtmcs >> UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & UU_VHT_MCSINDEX_FROM_MCS_MASK);

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Delta_ntx: %d, Delta_mcs: %d\n", delta_ntx, delta_mcs);
#endif

    /** Increase in TPC as per increase in mcs index */
    if (delta_mcs > 0)
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Printing uu_wlan_btpl_ac_g before changing as per delta_mcs\n");
        print_btpl();
#endif
        /** Increasing by delta_mcs levels */
        for (i = 0; i <= TPC_Max_Bandwidth; i++)
        {
            for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
            {
                uu_wlan_inc_BTPL_Level (delta_mcs, i, j);
            }
        }
    }

    /** Increase in TPC as per increase in number of spatial streams */
    if (delta_ntx > 0)
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Printing uu_wlan_btpl_ac_g before changing as per delta_ntx\n");
        print_btpl();
#endif
        for (i = 0; i <= TPC_Max_Bandwidth; i++)
        {
            for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
            {
                new_tpc_dBm_value = uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + j]]
                                    + (UU_WLAN_DBM_DOUBLING_FACTOR * CEIL(delta_ntx, ((normal_vhtmcs & 0x07) + 1)));

                uu_wlan_inc_BTPL_dBm (new_tpc_dBm_value, i, j);
            }
        }
    }
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Printing uu_wlan_btpl_ac_g after changing as per the new vht rate\n");
    print_btpl();
#endif
} /* uu_update_btpl_new_vhtmcs_all */


/**-------------------------------------------------------------------------------------
 * ----------------------------SPECIFIC FUNCTIONS---------------------------------------
 * ----------------------------UPDATING TFAR, WTFAR, WACTFAR----------------------------
 * ----------------------------UPDATING B_TPL AS PER WIDEBAND AC TFAR STATS-------------
 * ----------------------------RESETTING TFAR, WTFAR, WACTFAR---------------------------
 * -------------------------------------------------------------------------------------
 */

uu_void uu_wlan_calc_wtfar_stats (uu_uint8 bandwidth, uu_uint8 ac, uu_uint8 failed, uu_uint8 attempt)
{
    int i;

    bandwidth = (bandwidth == CBW80_80) ? CBW160 : bandwidth;
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk ("Bandwidth: %d, AC: %d, Attempt: %d, Failed: %d", bandwidth, ac, attempt, failed);
#endif
    if (attempt)
    {
        curr_prob = (failed << 12) / attempt;
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Current Probability: %d\n", curr_prob);
#endif
        if (attempt_hist)
        {
            tfar = (curr_prob * (100 - EWMA_LEVEL) + tfar * EWMA_LEVEL) / 100;
            wtfar[bandwidth] = (curr_prob * (100 - EWMA_LEVEL) + wtfar[bandwidth] * EWMA_LEVEL) / 100;
            wactfar[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] = (curr_prob * (100 - EWMA_LEVEL) +
                                                                wactfar[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] * EWMA_LEVEL) / 100;
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Cumulative TFAR: %d\n", tfar);
            printk("WTFAR for this bandwidth: %d\n", wtfar[bandwidth]);
            printk("WACTFAR for this ac and bandwidth: %d\n", wactfar[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]);
#endif
        }
        else
        {
            tfar = curr_prob;
            wtfar[bandwidth] = curr_prob;
            wactfar[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] = curr_prob;
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Cumulative TFAR: %d\n", tfar);
            printk("WTFAR for this bandwidth: %d\n", wtfar[bandwidth]);
            printk("WACTFAR for this ac and bandwidth: %d\n", wactfar[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]);
#endif
        }
    }

    attempt_hist += attempt;
    failed_hist += failed;

    /** Update B_TPL for the particular ac and the bandwidth for which the WAC-TFAR is updated */

    /** Implementation of Cumulative TFAR, Wideband TFAR, and Wideband AC TFAR.
     *
     * This Histogram provides the TFAR as per each AC and the bandwidth.
     * Thus, changes will be done only on the particular element per AC and bandwidth.
     *
     * Here we define 2 thresholds,
     *  a. WBAC_TFAR_Tmin: It represents the minimum TFAR for the current
     *      TPC, that shows best performance below which we consider decrementing TPC.
     *  b. WBAC_TFAR_Tmax: It represents the maximum TFAR above which we consider
     *      incrementing TPC.
     *
     * If  TFAR (AC/Bandwidth) < WBAC_TFAR_Tmin, decrease TPC by 3 levels for that
     *  particular AC as per the bandwidth.
     * If  TFAR (AC/Bandwidth) > WBAC_TFAR_Tmax, increase TPC by 5 levels for that
     *  particular AC as per the bandwidth.
     *
     * For 11n case, we increase TPC by 1 level or decrease TPC by 1 level for each AC/
     *  bandwidth.
     */

    /** If WACTFAR < TFAR_Min, then we will decrease TPL considering the lower limit */
    if (wactfar[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] < WAC_TFAR_Tmin)
    {
        /** For 2.4 GHz */
        if (uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] < UU_WLAN_TPLevel9)
        {
            uu_wlan_dec_BTPL_Level (TPC_WAC_TFAR_Ofdm_Dec, bandwidth, ac);
        }
        /** For 5 GHz */
        else
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] : %d\n", uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]);
            printk("uu_wlan_lowpload_datamgmt_ltpl[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] : %d\n", uu_wlan_lowpload_datamgmt_ltpl[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]);
            printk("TPC_WAC_TFAR_11ac_Dec : %d\n", TPC_WAC_TFAR_11ac_Dec);
#endif
            uu_wlan_dec_BTPL_Level (TPC_WAC_TFAR_11ac_Dec, bandwidth, ac);
        }
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Printing uu_wlan_btpl_ac_g after decreasing\n");
        print_btpl();
#endif
    }

    /** If WACTFAR > TFAR_Max, then we will increase TPL considering the upper limit */
    if (wactfar[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] > WAC_TFAR_Tmax)
    {
        /** For 2.4 GHz */
        if (uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] < UU_WLAN_TPLevel9)
        {
            uu_wlan_inc_BTPL_Level (TPC_WAC_TFAR_Ofdm_Inc, bandwidth, ac);
        }
        /** For 5 GHz */
        else
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac] : %d\n", uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + ac]);
            printk("uu_wlan_utpl_threshold_11ac_g[bandwidth] : %d\n", uu_wlan_utpl_threshold_11ac_g[bandwidth]);
            printk("TPC_WAC_TFAR_11ac_Inc : %d\n", TPC_WAC_TFAR_11ac_Inc);
#endif
            uu_wlan_inc_BTPL_Level (TPC_WAC_TFAR_11ac_Inc, bandwidth, ac);
        }
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Printing uu_wlan_btpl_ac_g after increasing\n");
        print_btpl();
#endif
    }
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Now printing statistics\n");
    printk("Cumulative TFAR: %d\n\n", tfar);

    printk("Wideband TFAR: ");
    for (i = 0; i <= TPC_Max_Bandwidth; i++)
    {
        printk("%d\t", wtfar[i]);
    }
    printk("\n");

    printk("Wideband AC TFAR (each row represents a single bandwidth)\n");
    for (i = 0; i <= TPC_Max_Bandwidth; i++)
    {
        printk("%d\t %d\t %d\t %d\t", wactfar[(i * UU_WLAN_AC_MAX_AC) + 0],
                                        wactfar[(i * UU_WLAN_AC_MAX_AC) + 1],
                                        wactfar[(i * UU_WLAN_AC_MAX_AC) + 2],
                                        wactfar[(i * UU_WLAN_AC_MAX_AC) + 3]);
        printk("\n");
    }
    printk("\n");
#endif
} /* uu_wlan_calc_wtfar_stats */


uu_void uu_wlan_reset_wtfar_stats (uu_void)
{
    int i, j;

    /** Resetting all statistics */
    for (i = 0; i <= TPC_Max_Bandwidth; i++)
    {
        for (j = 0; j < UU_WLAN_AC_MAX_AC; j++)
        {
            wactfar[(i * UU_WLAN_AC_MAX_AC) + j] = 0;
        }
        wtfar[i] = 0;
    }
    tfar = 0;
    attempt_hist = 0;
    failed_hist = 0;
    curr_prob = 0;

#ifdef UU_WLAN_DFSTPC_DEBUG
    /** Printing TODO: Remove all below lines after testing */
    printk("Cumulative TFAR: %d\n\n", tfar);

    printk("Wideband TFAR: ");
    for (i = 0; i <= TPC_Max_Bandwidth; i++)
    {
        printk("%d\t", wtfar[i]);
    }
    printk("\n");

    printk("Wideband AC TFAR (each row represents a single bandwidth)\n");
    for (i = 0; i <= TPC_Max_Bandwidth; i++)
    {
        printk("%d\t %d\t %d\t %d\t", wactfar[(i * UU_WLAN_AC_MAX_AC) + 0],
                                        wactfar[(i * UU_WLAN_AC_MAX_AC) + 1],
                                        wactfar[(i * UU_WLAN_AC_MAX_AC) + 2],
                                        wactfar[(i * UU_WLAN_AC_MAX_AC) + 3]);
        printk("\n");
    }
#endif
} /* uu_wlan_reset_wtfar_stats */

#endif /* UU_WLAN_TPC */

/* EOF */

