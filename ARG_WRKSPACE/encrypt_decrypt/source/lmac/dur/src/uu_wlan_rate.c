/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_rate.c                                         **
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
/* Contains functions and definitions for rate calculations. */

#include "uu_datatypes.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_rate.h"


/* Number of rows is 12, since registered hardware rates are total of CCK rates (4) and OFDM rates (8), so total comes 12 */
static const struct uu_legacy_rate_table  uu_lrate_table_g[] =
{
    /* bit_index, mbps, kbps250, hw_rate */
    /* CCK */

   /* code under UU_WLAN_RC_RTL_TABLE is for moving division functionality into scaler multiplication */
#ifdef UU_WLAN_RC_RTL_TABLE
   /* Modified the rate in 250kbps to (0xffffffffff/(rate in 250kbps)) in the bellow table, 40 bits in len */
    {0,     /* 1,  */	 274877906943ULL,  UU_HW_RATE_1M},  /* 1 Mbps */
    {1,     /* 2,  */    137438953471ULL,  UU_HW_RATE_2M},  /* 2 Mbps */
    {2,     /* 5.5,*/    49977801262ULL,   UU_HW_RATE_5M5}, /* 5.5 Mbps */
    {3,     /* 11, */    24988900631ULL,   UU_HW_RATE_11M}, /* 11 Mbps */
    /* OFDM */
    {4,     /* 6,  */    45812984490ULL,   UU_HW_RATE_6M},  /* 6 Mbps */
    {5,     /* 9,  */    30541989660ULL,   UU_HW_RATE_9M},  /* 9 Mbps */
    {6,     /* 12, */    22906492245ULL,   UU_HW_RATE_12M}, /* 12 Mbps */
    {7,     /* 18, */    15270994830ULL,   UU_HW_RATE_18M}, /* 18 Mbps */
    {8,     /* 24, */    11453246122ULL,   UU_HW_RATE_24M}, /* 24 Mbps */
    {9,     /* 36, */    7635497415ULL,    UU_HW_RATE_36M}, /* 36 Mbps */
    {10,    /* 48, */    5726623061ULL,    UU_HW_RATE_48M}, /* 48 Mbps */
    {11,    /* 54, */    5090331610ULL,    UU_HW_RATE_54M}, /* 54 Mbps */
#else
   /* Maintaing the lookup table for rate in 250kbps */
    /* CCK */
    {0,     /* 1,  */   4,      UU_HW_RATE_1M},  /* 1 Mbps */
    {1,     /* 2,  */   8,      UU_HW_RATE_2M},  /* 2 Mbps */
    {2,     /* 5.5,*/   22,     UU_HW_RATE_5M5}, /* 5.5 Mbps */
    {3,     /* 11, */   44,     UU_HW_RATE_11M}, /* 11 Mbps */
    /* OFDM */
    {4,     /* 6,  */   24,     UU_HW_RATE_6M},  /* 6 Mbps */
    {5,     /* 9,  */   36,     UU_HW_RATE_9M},  /* 9 Mbps */
    {6,     /* 12, */   48,     UU_HW_RATE_12M}, /* 12 Mbps */
    {7,     /* 18, */   72,     UU_HW_RATE_18M}, /* 18 Mbps */
    {8,     /* 24, */   96,     UU_HW_RATE_24M}, /* 24 Mbps */
    {9,     /* 36, */   144,    UU_HW_RATE_36M}, /* 36 Mbps */
    {10,    /* 48, */   192,    UU_HW_RATE_48M}, /* 48 Mbps */
    {11,    /* 54, */   216,    UU_HW_RATE_54M}, /* 54 Mbps */
#endif
}; /* legacy_rate_table  uu_lrate_table_g */

/* Number of rows is 32, since number of spatial stream supported = 4 and mcs index supported is 8. and unequal modulation is not supported. */
/* We define HT flags that takes only 5 bits and mask  higher bits, so it will always comes under this array */
/* Mcs index 32, which uses 1 spatial stream and 1 expansion spatial stream and only in 40 MHz. Since expansion stream is not supported then no need to include */
/* This is as per 802.11n standard REV_mb D12, section 20.6 */
const struct uu_ht_mcs_rate_table uu_ht_mcs_table_g[] =
{
    /* Note : Rate is in 250kbps multiple */
    /* mcs_index = indicates the MCS index. Keeping it in the table, just for clarity */
    /* mcs_20 = throughput we get for mcs index in 5th with Long GI in 20 MHz */
    /* mcs_40 = throughput we get for mcs index in 5th with Long GI in 40 MHz */
    /* mcs_20_sgi= throughput we get for mcs index in 5th with Short GI in 20 MHz */
    /* mcs_40_sgi = throughput we get for mcs index in 5th with Short GI in 40 MHz*/
    /* ofdm_rate = non-HT reference rate */
#ifdef UU_WLAN_RC_RTL_TABLE
    /* Modified the rate in 250kbps to (0xffffffffff/(rate in 250kbps)) in the bellow table, 40 bits in len */
    /* mcs_index, mcs_20, mcs_40, mcs_20_sgi, mcs_40_sgi, ofdm_rate */
    {0,     42288908760ULL, 20361326440ULL, 39268272420ULL, 18325193796ULL, UU_HW_RATE_6M},
    {1,     21144454380ULL, 10180663220ULL, 19289677680ULL, 9162596898ULL,  UU_HW_RATE_12M},
    {2,     14096302920ULL, 6787108813ULL,  12785018927ULL, 6108397932ULL,  UU_HW_RATE_18M},
    {3,     10572227190ULL, 5090331610ULL,  9560970676ULL,  4581298449ULL,  UU_HW_RATE_24M},
    {4,     7048151460ULL,  3020636340ULL,  6355558542ULL,  3054198966ULL,  UU_HW_RATE_36M},
    {5,     5286113595ULL,  2545165805ULL,  4759790596ULL,  2290649224ULL,  UU_HW_RATE_48M},
    {6,     4698767640ULL,  2262369604ULL,  4228890876ULL,  2036132644ULL,  UU_HW_RATE_54M},
    {7,     4228890876ULL,  2036132644ULL,  3817748707ULL,  1832519379ULL,  UU_HW_RATE_54M},

    {8,     21144454380ULL, 10180663220ULL, 19289677680ULL, 9162596898ULL,  UU_HW_RATE_6M},
    {9,     10572227190ULL, 5090331610ULL,  9560970676ULL,  4581298449ULL,  UU_HW_RATE_12M},
    {10,    7048151460ULL,  3393554406ULL,  6355558542ULL,  3054198966ULL,  UU_HW_RATE_18M},
    {11,    5286113595ULL,  2545165805ULL,  4759790596ULL,  2290649224ULL,  UU_HW_RATE_24M},
    {12,    3524075730ULL,  1696777203ULL,  3177779271ULL,  1527099483ULL,  UU_HW_RATE_36M},
    {13,    2643056797ULL,  1272582902ULL,  2379895298ULL,  1145324612ULL,  UU_HW_RATE_48M},
    {14,    2349383820ULL,  1131184802ULL,  2110387001ULL,  1018066322ULL,  UU_HW_RATE_54M},
    {15,    2114445438ULL,  1018066322ULL,  1905566079ULL,  916259689ULL,   UU_HW_RATE_54M},

    {16,    14096302920ULL, 6787108813ULL,  12785018927ULL, 6108397932ULL,  UU_HW_RATE_6M},
    {17,    7048151460ULL,  3393554406ULL,  6355558542ULL,  3054198966ULL,  UU_HW_RATE_12M},
    {18,    4698767640ULL,  2262369604ULL,  4228890876ULL,  2036132644ULL,  UU_HW_RATE_18M},
    {19,    3524075730ULL,  1696777203ULL,  3177779271ULL,  1527099483ULL,  UU_HW_RATE_24M},
    {20,    2349383820ULL,  1131184802ULL,  2114445438ULL,  1018066322ULL,  UU_HW_RATE_36M},
    {21,    1762037865ULL,  848388601ULL,   1586596865ULL,  763549741ULL,   UU_HW_RATE_48M},
    {22,    1566255880ULL,  754123201ULL,   1409630292ULL,  678710881ULL,   UU_HW_RATE_54M},
    {23,    1409630292ULL,  678710881ULL,   1269643911ULL,  610839793ULL,   UU_HW_RATE_54M},

    {24,    10572227190ULL, 5090331610ULL,  9560970676ULL,  4581298449ULL,  UU_HW_RATE_6M},
    {25,    5286113595ULL,  2545165805ULL,  4759790596ULL,  2290649224ULL,  UU_HW_RATE_12M},
    {26,    3524075730ULL,  1696777203ULL,  3177779271ULL,  1527099483ULL,  UU_HW_RATE_18M},
    {27,    2643056797ULL,  1272582902ULL,  2379895298ULL,  1145324612ULL,  UU_HW_RATE_24M},
    {28,    1762037865ULL,  848388601ULL,   1586596865ULL,  763549741ULL,   UU_HW_RATE_36M},
    {29,    1321528398ULL,  636291451ULL,   1189947649ULL,  572662306ULL,   UU_HW_RATE_48M},
    {30,    1174691910ULL,  565592401ULL,   1057222719ULL,  509033161ULL,   UU_HW_RATE_54M},
    {31,    1057222719ULL,  509033161ULL,   951958119ULL,   458129844ULL,   UU_HW_RATE_54M},
#else
    {0,    26,   54,   28,   60,  UU_HW_RATE_6M},
    {1,    52,  108,   57,  120,  UU_HW_RATE_12M},
    {2,    78,  162,   86,  180,  UU_HW_RATE_18M},
    {3,   104,  216,  115,  240,  UU_HW_RATE_24M},
    {4,   156,  364,  173,  360,  UU_HW_RATE_36M},
    {5,   208,  432,  231,  480,  UU_HW_RATE_48M},
    {6,   234,  486,  260,  540,  UU_HW_RATE_54M},
    {7,   260,  540,  288,  600,  UU_HW_RATE_54M},

    {8,    52,  108,   57,  120,  UU_HW_RATE_6M},
    {9,   104,  216,  115,  240,  UU_HW_RATE_12M},
    {10,  156,  324,  173,  360,  UU_HW_RATE_18M},
    {11,  208,  432,  231,  480,  UU_HW_RATE_24M},
    {12,  312,  648,  346,  720,  UU_HW_RATE_36M},
    {13,  416,  864,  462,  960,  UU_HW_RATE_48M},
    {14,  468,  972,  521, 1080,  UU_HW_RATE_54M},
    {15,  520, 1080,  577, 1200,  UU_HW_RATE_54M},

    {16,   78,  162,   86,  180,  UU_HW_RATE_6M},
    {17,  156,  324,  173,  360,  UU_HW_RATE_12M},
    {18,  234,  486,  260,  540,  UU_HW_RATE_18M},
    {19,  312,  648,  346,  720,  UU_HW_RATE_24M},
    {20,  468,  972,  520, 1080,  UU_HW_RATE_36M},
    {21,  624, 1296,  693, 1440,  UU_HW_RATE_48M},
    {22,  702, 1458,  780, 1620,  UU_HW_RATE_54M},
    {23,  780, 1620,  866, 1800,  UU_HW_RATE_54M},

    {24,  104,  216,  115,  240,  UU_HW_RATE_6M},
    {25,  208,  432,  231,  480,  UU_HW_RATE_12M},
    {26,  312,  648,  346,  720,  UU_HW_RATE_18M},
    {27,  416,  864,  462,  960,  UU_HW_RATE_24M},
    {28,  624, 1296,  693, 1440,  UU_HW_RATE_36M},
    {29,  832, 1728,  924, 1920,  UU_HW_RATE_48M},
    {30,  936, 1944, 1040, 2160,  UU_HW_RATE_54M},
    {31, 1040, 2160, 1155, 2400,  UU_HW_RATE_54M},
#endif
}; /* uu_ht_mcs_rate_table uu_ht_mcs_table_g */
EXPORT_SYMBOL(uu_ht_mcs_table_g);

/* Number of rows is 80, since number of spatial stream supported = 8 and mcs index supported is 10. So total comes 80 */
/* This is as per 802.11ac standard D3.0, section 22.5 */
const struct uu_vht_mcs_rate_table uu_vht_mcs_table_g[] =
{
    /* Note : Rate is in 250kbps multiple */
    /* mcs_nss_index_ac = indicates the VHT MCS index */
    /* mcs_ac_20 = throughput we get for vht mcs index in 5th with Long GI in 20 MHz */
    /* mcs_ac_40 = throughput we get for vht mcs index in 5th with Long GI in 40 MHz */
    /* mcs_ac_80 = throughput we get for vht mcs index in 5th with Long GI in 80 MHz */
    /* mcs_ac_160 = throughput we get for vht mcs index in 5th with Long GI in 160 MHz */
    /* mcs_ac_20_sgi= throughput we get for vht mcs index in 5th with Short GI in 20 MHz */
    /* mcs_ac_40_sgi = throughput we get for vht mcs index in 5th with Short GI in 40 MHz*/
    /* mcs_ac_20_sgi= throughput we get for vht mcs index in 5th with Short GI in 80 MHz */
    /* mcs_ac_40_sgi = throughput we get for vht mcs index in 5th with Short GI in 160 MHz*/
    /* ofdm_rate = non-HT reference rate */

    /*mcs_index    mcs_ac_40,    mcs_ac_160,    mcs_ac_40_sgi,     mcs_ac_160_sgi,    ofdm_rate */
    /*     mcs_ac_20,   mcs_ac_80,      mcs_ac_20_sgi,     mcs_ac_80_sgi, */
#ifdef UU_WLAN_RC_RTL_TABLE
    /* Modified the rate in 250kbps to (0xffffffffff/(rate in 250kbps)) in the bellow table, 40 bits in len */
    {0,	    42288908760ULL, 20361326440ULL, 9397535280ULL,  4698767640ULL,  39268272420ULL, 18325193796ULL, 8457781752ULL,  4228890876ULL,  UU_HW_RATE_6M},	
    {1,	    21144454380ULL, 10180663220ULL, 4698767640ULL,  2349383820ULL,  19289677680ULL, 9162596898ULL,  4228890876ULL,  2114445438ULL,  UU_HW_RATE_12M},
    {2,	    14096302920ULL, 6787108813ULL,  3132511760ULL,  1566255880ULL,  12785018927ULL, 6108397932ULL,  2819260584ULL,  1409630292ULL,  UU_HW_RATE_18M},
    {3,	    10572227190ULL, 5090331610ULL,  2349383820ULL,  1174691910ULL,  9560970676ULL,  4581298449ULL,  2114445438ULL,  1057222719ULL,  UU_HW_RATE_24M},
    {4,	    7048151460ULL,  3393554406ULL,  1566255880ULL,  783127940ULL,   6355558542ULL,  3054198966ULL,  1409630292ULL,  704815146ULL,   UU_HW_RATE_36M},
    {5,	    5286113595ULL,  2545165805ULL,  1174691910ULL,  587345955ULL,   4759790596ULL,  2290649224ULL,  1057222719ULL,  528611359ULL,   UU_HW_RATE_48M},
    {6,	    4698767640ULL,  2262369604ULL,  1044170586ULL,  522085293ULL,   4228890876ULL,  2036132644ULL,  939753528ULL,   469876764ULL,   UU_HW_RATE_54M},
    {7,	    4228890876ULL,  2036132644ULL,  939753528ULL,   469876764ULL,   3817748707ULL,  1832519379ULL,  845778175ULL,   422889087ULL,   UU_HW_RATE_54M},
    {8,	    3524075730ULL,  1696777203ULL,  783127940ULL,   391563970ULL,   3177779271ULL,  1527099483ULL,  704815146ULL,   352407573ULL,   UU_HW_RATE_54M},
    {9,	    4294967295ULL,  1527099483ULL,  704815146ULL,   352407573ULL,   4294967295ULL,  1374389534ULL,  634455642ULL,   317227821ULL,   UU_HW_RATE_54M},

    {10,    21144454380ULL, 10180663220ULL, 4698767640ULL,  2349383820ULL,  19289677680ULL, 9162596898ULL,  4228890876ULL,  2114445438ULL,  UU_HW_RATE_6M},
    {11,    10572227190ULL, 5090331610ULL,  2349383820ULL,  1174691910ULL,  9560970676ULL,  4581298449ULL,  2114445438ULL,  1057222719ULL,  UU_HW_RATE_12M},
    {12,    7048151460ULL,  3393554406ULL,  1566255880ULL,  783127940ULL,   6355558542ULL,  3054198966ULL,  1409630292ULL,  704815146ULL,   UU_HW_RATE_18M}, 
    {13,    5286113595ULL,  2545165805ULL,  1174691910ULL,  587345955ULL,   4759790596ULL,  2290649224ULL,  1057222719ULL,  528611359ULL,   UU_HW_RATE_24M},
    {14,    3524075730ULL,  1696777203ULL,  783127940ULL,   391563970ULL,   3177779271ULL,  1527099483ULL,  704815146ULL,   352407573ULL,   UU_HW_RATE_36M},
    {15,    2643056797ULL,  1272582902ULL,  587345955ULL,   293672977ULL,   2379895298ULL,  1145324612ULL,  528611359ULL,   264305679ULL,   UU_HW_RATE_48M},
    {16,    2349383820ULL,  1131184802ULL,  522085293ULL,   261042646ULL,   2114445438ULL,  1018066322ULL,  469876764ULL,   234938382ULL,   UU_HW_RATE_54M},
    {17,    2114445438ULL,  1018066322ULL,  469876764ULL,   234938382ULL,   1905566079ULL,  916259689ULL,   422889087ULL,   211444543ULL,   UU_HW_RATE_54M},
    {18,    1762037865ULL,  848388601ULL,   391563970ULL,   195781985ULL,   1586596865ULL,  763549741ULL,   352407573ULL,   176203786ULL,   UU_HW_RATE_54M},
    {19,    4294967295ULL,  763549741ULL,   352407573ULL,   176203786ULL,   4294967295ULL,  687194767ULL,   317227821ULL,   158591032ULL,   UU_HW_RATE_54M},

    {20,    14096302920ULL, 6787108813ULL,  3132511760ULL,  1566255880ULL,  12785018927ULL, 6108397932ULL,  2819260584ULL,  1409630292ULL,  UU_HW_RATE_6M},
    {21,    7048151460ULL,  3393554406ULL,  1566255880ULL,  783127940ULL,   6355558542ULL,  3054198966ULL,  1409630292ULL,  704815146ULL,   UU_HW_RATE_12M},
    {22,    4698767640ULL,  2262369604ULL,  1044170586ULL,  522085293ULL,   4228890876ULL,  2036132644ULL,  939753528ULL,   469876764ULL,   UU_HW_RATE_18M},
    {23,    3524075730ULL,  1696777203ULL,  783127940ULL,   391563970ULL,   3177779271ULL,  1527099483ULL,  704815146ULL,   352407573ULL,   UU_HW_RATE_24M},
    {24,    2349383820ULL,  1131184802ULL,  522085293ULL,   261042646ULL,   2114445438ULL,  1018066322ULL,  469876764ULL,   234938382ULL,   UU_HW_RATE_36M},
    {25,    1762037865ULL,  848388601ULL,   391563970ULL,   195781985ULL,   1586596865ULL,  763549741ULL,   352407573ULL,   176203786ULL,   UU_HW_RATE_48M},
    {26,    1566255880ULL,  754123201ULL,   4294967295ULL,  174028431ULL,   1409630292ULL,  678710881ULL,   4294967295ULL,  156625588ULL,   UU_HW_RATE_54M},
    {27,    1409630292ULL,  678710881ULL,   313251176ULL,   156625588ULL,   1269643911ULL,  610839793ULL,   281926058ULL,   140963029ULL,   UU_HW_RATE_54M},
    {28,    1174691910ULL,  565592401ULL,   261042646ULL,   130521323ULL,   1057222719ULL,  509033161ULL,   234938382ULL,   117469191ULL,   UU_HW_RATE_54M},
    {29,    1057222719ULL,  509033161ULL,   234938382ULL,   4294967295ULL,  951958119ULL,   458129844ULL,   211444543ULL,   4294967295ULL,  UU_HW_RATE_54M},

    {30,    10572227190ULL, 5090331610ULL,  2349383820ULL,  1174691910ULL,  9560970676ULL,  4581298449ULL,  2114445438ULL,  10572227190ULL, UU_HW_RATE_6M},
    {31,    5286113595ULL,  2545165805ULL,  1174691910ULL,  587345955ULL,   4759790596ULL,  2290649224ULL,  1057222719ULL,  528611359ULL,   UU_HW_RATE_12M},
    {32,    3524075730ULL,  1696777203ULL,  783127940ULL,   391563970ULL,   3177779271ULL,  1527099483ULL,  704815146ULL,   352407573ULL,   UU_HW_RATE_18M},
    {33,    2643056797ULL,  1272582902ULL,  587345955ULL,   293672977ULL,   2379895298ULL,  1145324612ULL,  528611359ULL,   264305679ULL,   UU_HW_RATE_24M},
    {34,    1762037865ULL,  848388601ULL,   391563970ULL,   195781985ULL,   1586596865ULL,  763549741ULL,   352407573ULL,   176203786ULL,   UU_HW_RATE_36M},
    {35,    1321528398ULL,  636291451ULL,   293672977ULL,   146836488ULL,   1189947649ULL,  572662306ULL,   264305679ULL,   132152839ULL,   UU_HW_RATE_48M},
    {36,    1174691910ULL,  565592401ULL,   261042646ULL,   130521323ULL,   1057222719ULL,  509033161ULL,   234938382ULL,   117469191ULL,   UU_HW_RATE_54M},
    {37,    1057222719ULL,  509033161ULL,   234938382ULL,   117469191ULL,   951958119ULL,   458129844ULL,   211444543ULL,   105722271ULL,   UU_HW_RATE_54M},
    {38,    881018932ULL,   424194300ULL,   195781985ULL,   97890992ULL,    793298432ULL,   381774870ULL,   176203786ULL,   88101893ULL,    UU_HW_RATE_54M},
    {39,    4294967295ULL,  381774870ULL,   176203786ULL,   88101893ULL,    4294967295ULL,  343597383ULL,   158591032ULL,   79295516ULL,    UU_HW_RATE_54M},

    {40,    8457781752ULL,  4072265288ULL,  1879507056ULL,  939753528ULL,   7635497415ULL,  3665038759ULL,  1691556350ULL,  845778175ULL,   UU_HW_RATE_6M},
    {41,    4228890876ULL,  2036132644ULL,  939753528ULL,   469876764ULL,   3817748707ULL,  1832519379ULL,  845778175ULL,   422889087ULL,   UU_HW_RATE_12M},
    {42,    2819260584ULL,  1357421762ULL,  626502352ULL,   313251176ULL,   2539287823ULL,  1221679586ULL,  563852116ULL,   281926058ULL,   UU_HW_RATE_18M},
    {43,    2114445438ULL,  1018066322ULL,  469876764ULL,   234938382ULL,   1905566079ULL,  916259689ULL,   422889087ULL,   211444543ULL,   UU_HW_RATE_24M},
    {44,    1409630292ULL,  678710881ULL,   313251176ULL,   156625588ULL,   1269643911ULL,  610839793ULL,   281926058ULL,   140963029ULL,   UU_HW_RATE_36M},
    {45,    1057222719ULL,  509033161ULL,   234938382ULL,   117469191ULL,   951958119ULL,   458129844ULL,   211444543ULL,   105722271ULL,   UU_HW_RATE_48M},
    {46,    939753528ULL,   452473920ULL,   208834117ULL,   104417058ULL,   845778175ULL,   407226528ULL,   187950705ULL,   93975352ULL,    UU_HW_RATE_54M},
    {47,    845778175ULL,   407226528ULL,   187950705ULL,   93975352ULL,    761434645ULL,   366503875ULL,   169155635ULL,   84577817ULL,    UU_HW_RATE_54M},
    {48,    704815146ULL,   339355440ULL,   156625588ULL,   78312794ULL,    634455642ULL,   305419896ULL,   140963029ULL,   70481514ULL,    UU_HW_RATE_54M},
    {49,    4294967295ULL,  305419896ULL,   140963029ULL,   70481514ULL,    4294967295ULL,  274877906ULL,   126876486ULL,   63434583ULL,    UU_HW_RATE_54M},

    {50,    7048151460ULL,  3393554406ULL,  1566255880ULL,  783127940ULL,   6355558542ULL,  3054198966ULL,  1409630292ULL,  704815146ULL,   UU_HW_RATE_6M},
    {51,    3524075730ULL,  1696777203ULL,  783127940ULL,   391563970ULL,   3177779271ULL,  1527099483ULL,  704815146ULL,   352407573ULL,   UU_HW_RATE_12M},
    {52,    2349383820ULL,  1131184802ULL,  522085293ULL,   261042646ULL,   2114445438ULL,  1018066322ULL,  469876764ULL,   234938382ULL,   UU_HW_RATE_18M},
    {53,    1762037865ULL,  848388601ULL,   391563970ULL,   195781985ULL,   1586596865ULL,  763549741ULL,   352407573ULL,   176203786ULL,   UU_HW_RATE_24M},
    {54,    1174691910ULL,  565592401ULL,   261042646ULL,   130521323ULL,   1057222719ULL,  509033161ULL,   234938382ULL,   117469191ULL,   UU_HW_RATE_36M},
    {55,    881018932ULL,   424194300ULL,   195781985ULL,   97890992ULL,    793298432ULL,   381774870ULL,   176203786ULL,   88101893ULL,    UU_HW_RATE_48M},
    {56,    783127940ULL,   377061600ULL,   174028431ULL,   87014215ULL,    704815146ULL,   339355440ULL,   156625588ULL,   78312794ULL,    UU_HW_RATE_54M},
    {57,    704815146ULL,   339355440ULL,   156625588ULL,   78312794ULL,    634455642ULL,   305419896ULL,   140963029ULL,   70481514ULL,    UU_HW_RATE_54M},
    {58,    587345955ULL,   282796200ULL,   130521323ULL,   65260661ULL,    528611359ULL,   254516580ULL,   117469191ULL,   58734595ULL,    UU_HW_RATE_54M},
    {59,    528611359ULL,   254516580ULL,   4294967295ULL,  58734595ULL,    475773097ULL,   229064922ULL,   4294967295ULL,  52861135ULL,    UU_HW_RATE_54M},

    {60,    6041272680ULL,  2908760920ULL,  1342505040ULL,  671252520ULL,   5443126870ULL,  2617884828ULL,  1208254536ULL,  604127268ULL,   UU_HW_RATE_6M},
    {61,    3020636340ULL,  1454380460ULL,  671252520ULL,   335626260ULL,   2721563435ULL,  1308942414ULL,  604127268ULL,   302063634ULL,   UU_HW_RATE_12M},
    {62,    2013757560ULL,  969586973ULL,   447501680ULL,   223750840ULL,   1814375623ULL,  872628276ULL,   402751512ULL,   201375756ULL,   UU_HW_RATE_18M},
    {63,    1510318170ULL,  727190230ULL,   335626260ULL,   167813130ULL,   1360781717ULL,  654471207ULL,   302063634ULL,   151031817ULL,   UU_HW_RATE_24M},
    {64,    1006878780ULL,  484793486ULL,   223750840ULL,   111875420ULL,   906439923ULL,   436314138ULL,   201375756ULL,   100687878ULL,   UU_HW_RATE_36M},
    {65,    755159085ULL,   363595115ULL,   167813130ULL,   83906565ULL,    679970085ULL,   327235603ULL,   151031817ULL,   75515908ULL,    UU_HW_RATE_48M},
    {66,    671252520ULL,   323195657ULL,   4294967295ULL,  74583613ULL,    604127268ULL,   290876092ULL,   4294967295ULL,  67125252ULL,    UU_HW_RATE_54M},
    {67,    604127268ULL,   290876092ULL,   134250504ULL,   67125252ULL,    543774296ULL,   261788482ULL,   120825453ULL,   60412726ULL,    UU_HW_RATE_54M},
    {68,    503439390ULL,   242396743ULL,   111875420ULL,   55937710ULL,    453219961ULL,   218157069ULL,   100687878ULL,   50343939ULL,    UU_HW_RATE_54M},
    {69,    4294967295ULL,  218157069ULL,   100687878ULL,   50343939ULL,    4294967295ULL,  196341362ULL,   90621579ULL,    45310789ULL,    UU_HW_RATE_54M},

    {70,    5286113595ULL,  2545165805ULL,  1174691910ULL,  587345955ULL,   4759790596ULL,  2290649224ULL,  1057222719ULL,  528611359ULL,   UU_HW_RATE_6M},
    {71,    2643056797ULL,  1272582902ULL,  587345955ULL,   293672977ULL,   2379895298ULL,  1145324612ULL,  528611359ULL,   264305679ULL,   UU_HW_RATE_12M},
    {72,    1762037865ULL,  848388601ULL,   391563970ULL,   195781985ULL,   1586596865ULL,  763549741ULL,   352407573ULL,   176203786ULL,   UU_HW_RATE_18M},
    {73,    1321528398ULL,  636291451ULL,   293672977ULL,   146836488ULL,   1189947649ULL,  572662306ULL,   264305679ULL,   132152839ULL,   UU_HW_RATE_24M},
    {74,    881018932ULL,   424194300ULL,   195781985ULL,   97890992ULL,    793298432ULL,   381774870ULL,   176203786ULL,   88101893ULL,    UU_HW_RATE_36M},
    {75,    660764199ULL,   318145725ULL,   146836488ULL,   73418244ULL,    594973824ULL,   286331153ULL,   132152839ULL,   66076419ULL,    UU_HW_RATE_48M},
    {76,    587345955ULL,   282796200ULL,   130521323ULL,   65260661ULL,    528611359ULL,   254516580ULL,   117469191ULL,   58734595ULL,    UU_HW_RATE_54M},
    {77,    528611359ULL,   254516580ULL,   117469191ULL,   58734595ULL,    475773097ULL,   229064922ULL,   105722271ULL,   52861135ULL,    UU_HW_RATE_54M},
    {78,    440509466ULL,   212097150ULL,   97890992ULL,    48945496ULL,    396506176ULL,   190887435ULL,   88101893ULL,    44050946ULL,    UU_HW_RATE_54M},
    {79,    4294967295ULL,  190887435ULL,   88101893ULL,    44050946ULL,    4294967295ULL,  171798691ULL,   79295516ULL,    39646328ULL,    UU_HW_RATE_54M},

#else
    {0,     26,     54,    117,    234,    28,     60,    130,    260,   UU_HW_RATE_6M},
    {1,     52,    108,    234,    468,    57,    120,    260,    520,   UU_HW_RATE_12M},
    {2,     78,    162,    351,    702,    86,    180,    390,    780,   UU_HW_RATE_18M},
    {3,    104,    216,    468,    936,   115,    240,    520,   1040,   UU_HW_RATE_24M},
    {4,    156,    324,    702,   1404,   173,    360,    780,   1560,   UU_HW_RATE_36M},
    {5,    208,    432,    936,   1872,   231,    480,   1040,   2080,   UU_HW_RATE_48M},
    {6,    234,    486,   1053,   2106,   260,    540,   1170,   2340,   UU_HW_RATE_54M},
    {7,    260,    540,   1170,   2340,   288,    600,   1300,   2600,   UU_HW_RATE_54M},
    {8,    312,    648,   1404,   2808,   346,    720,   1560,   3120,   UU_HW_RATE_54M},
    {9,      0,    720,   1560,   3120,     0,    800,   1733,   3466,   UU_HW_RATE_54M},

    {10,     52,   108,    234,    468,    57,    120,    260,    520,   UU_HW_RATE_6M},
    {11,    104,   216,    468,    936,   115,    240,    520,   1040,   UU_HW_RATE_12M},
    {12,    156,   324,    702,   1404,   173,    360,    780,   1560,   UU_HW_RATE_18M},
    {13,    208,   432,    936,   1872,   231,    480,   1040,   2080,   UU_HW_RATE_24M},
    {14,    312,   648,   1404,   2808,   346,    720,   1560,   3120,   UU_HW_RATE_36M},
    {15,    416,   864,   1872,   3744,   462,    960,   2080,   4160,   UU_HW_RATE_48M},
    {16,    468,   972,   2106,   4212,   520,   1080,   2340,   4680,   UU_HW_RATE_54M},
    {17,    520,  1080,   2340,   4680,   577,   1200,   2600,   5200,   UU_HW_RATE_54M},
    {18,    624,  1296,   2808,   5616,   693,   1440,   3120,   6240,   UU_HW_RATE_54M},
    {19,      0,  1440,   3120,   6240,     0,   1600,   3466,   6933,   UU_HW_RATE_54M},

    {20,     78,   162,    351,    702,    86,    180,    390,    780,   UU_HW_RATE_6M},
    {21,    156,   324,    702,   1404,   173,    360,    780,   1560,   UU_HW_RATE_12M},
    {22,    234,   486,   1053,   2106,   260,    540,   1170,   2340,   UU_HW_RATE_18M},
    {23,    312,   648,   1404,   2808,   346,    720,   1560,   3120,   UU_HW_RATE_24M},
    {24,    468,   972,   2106,   4212,   520,   1080,   2340,   4680,   UU_HW_RATE_36M},
    {25,    624,  1296,   2808,   5616,   693,   1440,   3120,   6240,   UU_HW_RATE_48M},
    {26,    702,  1458,      0,   6318,   780,   1620,      0,   7020,   UU_HW_RATE_54M},
    {27,    780,  1620,   3510,   7020,   866,   1800,   3900,   7800,   UU_HW_RATE_54M},
    {28,    936,  1944,   4212,   8424,  1040,   2160,   4680,   9360,   UU_HW_RATE_54M},
    {29,   1040,  2160,   4680,      0,  1155,   2400,   5200,      0,   UU_HW_RATE_54M},

    {30,    104,   216,    468,    936,   115,    240,    520,    104,   UU_HW_RATE_6M},
    {31,    208,   432,    936,   1872,   231,    480,   1040,   2080,   UU_HW_RATE_12M},
    {32,    312,   648,   1404,   2808,   346,    720,   1560,   3120,   UU_HW_RATE_18M},
    {33,    416,   864,   1872,   3744,   462,    960,   2080,   4160,   UU_HW_RATE_24M},
    {34,    624,  1296,   2808,   5616,   693,   1440,   3120,   6240,   UU_HW_RATE_36M},
    {35,    832,  1728,   3744,   7488,   924,   1920,   4160,   8320,   UU_HW_RATE_48M},
    {36,    936,  1944,   4212,   8424,  1040,   2160,   4680,   9360,   UU_HW_RATE_54M},
    {37,   1040,  2160,   4680,   9360,  1155,   2400,   5200,  10400,   UU_HW_RATE_54M},
    {38,   1248,  2592,   5616,  11232,  1386,   2880,   6240,  12480,   UU_HW_RATE_54M},
    {39,      0,  2880,   6240,  12480,     0,   3200,   6933,  13866,   UU_HW_RATE_54M},

    {40,    130,   270,    585,   1170,   144,    300,    650,   1300,   UU_HW_RATE_6M},
    {41,    260,   540,   1170,   2340,   288,    600,   1300,   2600,   UU_HW_RATE_12M},
    {42,    390,   810,   1755,   3510,   433,    900,   1950,   3900,   UU_HW_RATE_18M},
    {43,    520,  1080,   2340,   4680,   577,   1200,   2600,   5200,   UU_HW_RATE_24M},
    {44,    780,  1620,   3510,   7020,   866,   1800,   3900,   7800,   UU_HW_RATE_36M},
    {45,   1040,  2160,   4680,   9360,  1155,   2400,   5200,  10400,   UU_HW_RATE_48M},
    {46,   1170,  2430,   5265,  10530,  1300,   2700,   5850,  11700,   UU_HW_RATE_54M},
    {47,   1300,  2700,   5850,  11700,  1444,   3000,   6500,  13000,   UU_HW_RATE_54M},
    {48,   1560,  3240,   7020,  14040,  1733,   3600,   7800,  15600,   UU_HW_RATE_54M},
    {49,      0,  3600,   7800,  15600,     0,   4000,   8666,  17333,   UU_HW_RATE_54M},

    {50,    156,   324,    702,   1404,   173,    360,    780,   1560,   UU_HW_RATE_6M},
    {51,    312,   648,   1404,   2808,   346,    720,   1560,   3120,   UU_HW_RATE_12M},
    {52,    468,   972,   2106,   4212,   520,   1080,   2340,   4680,   UU_HW_RATE_18M},
    {53,    624,  1296,   2808,   5616,   693,   1440,   3120,   6240,   UU_HW_RATE_24M},
    {54,    936,  1944,   4212,   8424,  1040,   2160,   4680,   9360,   UU_HW_RATE_36M},
    {55,   1248,  2592,   5616,  11232,  1386,   2880,   6240,  12480,   UU_HW_RATE_48M},
    {56,   1404,  2916,   6318,  12636,  1560,   3240,   7020,  14040,   UU_HW_RATE_54M},
    {57,   1560,  3240,   7020,  14040,  1733,   3600,   7800,  15600,   UU_HW_RATE_54M},
    {58,   1872,  3888,   8424,  16848,  2080,   4320,   9360,  18720,   UU_HW_RATE_54M},
    {59,   2080,  4320,      0,  18720,  2311,   4800,      0,  20800,   UU_HW_RATE_54M},

    {60,    182,   378,    819,   1638,   202,    420,    910,   1820,   UU_HW_RATE_6M},
    {61,    364,   756,   1638,   3276,   404,    840,   1820,   3640,   UU_HW_RATE_12M},
    {62,    546,  1134,   2457,   4914,   606,   1260,   2730,   5460,   UU_HW_RATE_18M},
    {63,    728,  1512,   3276,   6552,   808,   1680,   3640,   7280,   UU_HW_RATE_24M},
    {64,   1092,  2268,   4914,   9828,  1213,   2520,   5460,  10920,   UU_HW_RATE_36M},
    {65,   1456,  3024,   6552,  13104,  1617,   3360,   7280,  14560,   UU_HW_RATE_48M},
    {66,   1638,  3402,      0,  14742,  1820,   3780,      0,  16380,   UU_HW_RATE_54M},
    {67,   1820,  3780,   8190,  16380,  2022,   4200,   9100,  18200,   UU_HW_RATE_54M},
    {68,   2184,  4536,   9828,  19656,  2426,   5040,  10920,  21840,   UU_HW_RATE_54M},
    {69,      0,  5040,  10920,  21840,     0,   5600,  12133,  24266,   UU_HW_RATE_54M},

    {70,    208,   432,    936,   1872,   231,    480,   1040,   2080,   UU_HW_RATE_6M},
    {71,    416,   864,   1872,   3744,   462,    960,   2080,   4160,   UU_HW_RATE_12M},
    {72,    624,  1296,   2808,   5616,   693,   1440,   3120,   6240,   UU_HW_RATE_18M},
    {73,    832,  1728,   3744,   7488,   924,   1920,   4160,   8320,   UU_HW_RATE_24M},
    {74,   1248,  2592,   5616,  11232,  1386,   2880,   6240,  12480,   UU_HW_RATE_36M},
    {75,   1664,  3456,   7488,  14976,  1848,   3840,   8320,  16640,   UU_HW_RATE_48M},
    {76,   1872,  3888,   8424,  16848,  2080,   4320,   9360,  18720,   UU_HW_RATE_54M},
    {77,   2080,  4320,   9360,  18720,  2311,   4800,  10400,  20800,   UU_HW_RATE_54M},
    {78,   2496,  5184,  11232,  22464,  2773,   5760,  12480,  24960,   UU_HW_RATE_54M},
    {79,      0,  5760,  12480,  24960,     0,   6400,  13866,  27733,   UU_HW_RATE_54M},
#endif
}; /* uu_vht_mcs_rate_table uu_vht_mcs_table_g */
EXPORT_SYMBOL(uu_vht_mcs_table_g);


/* Position specifies as per uu_lrate_table_g */
static const uu_uint8 uu_legacy_ofdm_rate_to_index_map_g[UU_SUPPORTED_OFDM_RATE] =
{
    /* pos */
    /* For OFDM refer to 18.3.4.2 802.11-REVmbD12 */
    /* R4-R1: 1011 for 6 Mbps, After masking : 0011 (3), So bit position is 4 in uu_lrate_table_g */
    /* R4-R1: 1111 for 9 Mbps, After masking : 0111 (7), So bit position is 5 in uu_lrate_table_g */
    /* R4-R1: 1010 for 12 Mbps, After masking : 0010 (2), So bit position is 6 in uu_lrate_table_g */
    /* R4-R1: 1110 for 18 Mbps, After masking : 0110 (6), So bit position is 7 in uu_lrate_table_g */
    /* R4-R1: 1001 for 24 Mbps, After masking : 0001 (1), So bit position is 8 in uu_lrate_table_g */
    /* R4-R1: 1101 for 36 Mbps, After masking : 0101 (5), So bit position is 9 in uu_lrate_table_g */
    /* R4-R1: 1000 for 48 Mbps, After masking : 0000 (0), So bit position is 10 in uu_lrate_table_g */
    /* R4-R1: 1100 for 54 Mbps, After masking : 0100 (4), So bit position is 11 in uu_lrate_table_g */
    10, 8, 6, 4, 11, 9, 7, 5
}; /* uu_uint8 uu_legacy_ofdm_rate_to_index_map_g */


/* Returns the bit index as per the hardware rate */
static uu_int32 lmac_rate_to_index (uu_uint8 lmac_rate)
{
    /* OFDM rate */
    if (lmac_rate & UU_OFDM_RATE_FLAG)
    {
        return uu_legacy_ofdm_rate_to_index_map_g[(lmac_rate & UU_OFDM_HW_MASK_INDEX)];
    }
    else /* CCK rate */
    {
        uu_uint8 pos;
        if(lmac_rate == UU_HW_RATE_11M)
            pos = 3;
        else if(lmac_rate == UU_HW_RATE_5M5)
            pos = 2;
        else if(lmac_rate == UU_HW_RATE_2M)
            pos = 1;
        else /* UU_HW_RATE_1M */
            pos = 0;
        return pos;
    }
} /* lmac_rate_to_index */

/* This function returns the basic rate from BSSBasicRateSet which is <= legacy_rate */
static int get_basic_rate_legacy(int l_rate)
{
    /* For OFDM, if we get result = 0 then we won't consider. But for CCK it is valid */
    int result = UU_RATE_1M_BIT;
    int i;

    /* WARNING: Flag 0x80 is proprietary implementation. It indicates CCK */
    if (l_rate < UU_SUPPORTED_CCK_RATE)
    {
        i = UU_RATE_1M_BIT;
    }
    else
    {
        i = UU_RATE_6M_BIT;
    }

    /* Match the list of rates to get highest possible rate */
    for (; i <= l_rate; i++)
    {
        if (!(BSSBasicRateSet & BIT(i)))
            /* If the bit of the rate is not in basic rate then it will continue to check for the next */
            continue;
        /* If the bit of the rate is in basic rate, then it will keep checking for the higher basic rate
           less than the legacy rate, if it won't get then returns this one */
        result = i; /* Get the highest rate */
    }
    /* Returns the bit that corresponds to the basic rate */
    return result;
} /* get_basic_rate_legacy */


/* Returns the basic rate required for the calculation of duration for control response frames and other control frames */
uu_uint8 uu_assign_basic_rate (uu_uint8 modulation_type, uu_uint8 frame_format, uu_uint8 lmac_rate)
{
    uu_uint8 basic_rate = lmac_rate;
    int l_index, get_blrate;

    if(frame_format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        /* Returns the non-HT reference OFDM rate for the corresponding VHT MCS */
        lmac_rate = uu_vht_mcs_table_g[lmac_rate].ofdm_rate;
    }
    else if (frame_format == UU_WLAN_FRAME_FORMAT_HT_MF)
    {
        lmac_rate = uu_ht_mcs_table_g[lmac_rate & UU_HT_MCS_MASK].ofdm_rate; /* Non-HT reference corresponding to the rate as per section 9.7.9 */
    }

    /* TODO : for L-SIG Protection, we assign basic rate same as for HT-GF */

    else if(frame_format == UU_WLAN_FRAME_FORMAT_HT_GF)
    {
        basic_rate = (lmac_rate & UU_VHT_NSS_FROM_MCS_MASK); /* MCS index WITH 1 SPATIAL STREAM */
        return basic_rate;
    }

    /* Using BSSBasicRateSet Parameter */
    /* This will be of 16 bits, ie each bit representing legacy rates, i.e., 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 Mbps */
    /* If contained in basic rate, then the corresponding bit will be 1 else 0 */
    /* This function will be for letting to choose basic_rate for control response frames */
    /* returns the BIT index of the legacy rate */
    l_index = lmac_rate_to_index(lmac_rate);

    /* Now we search for the basic rate for the legacy rate */
    get_blrate = get_basic_rate_legacy(l_index);

    /* If rate is contained in BSSBasicRateSet, then get the hw_value of the rate */
    if (get_blrate)
    {
        basic_rate = uu_lrate_table_g[get_blrate].hw_rate;
    }
    /* Set the mandatory rates as per the modulation type */
    else
    {
        if ((modulation_type == ERP_DSSS) && (l_index <= UU_RATE_2M_BIT))
        {
            if (lmac_rate == UU_HW_RATE_1M)
            {
                basic_rate = UU_HW_RATE_1M;
            }
            else
            {
                basic_rate = UU_HW_RATE_2M;
            }
        }
        else if ((modulation_type == ERP_CCK || modulation_type == ERP_PBCC) && (l_index < UU_SUPPORTED_CCK_RATE))
        {
            if ((lmac_rate == UU_HW_RATE_1M) || (lmac_rate == UU_HW_RATE_2M) || (lmac_rate == UU_HW_RATE_5M5))
            {
                basic_rate = UU_HW_RATE_5M5;
            }
            else
            {
                basic_rate = UU_HW_RATE_11M;
            }
        }
        /* For OFDM refer to 18.3.4.2 802.11-REVmbD12 */
        /* R4-R1 : 1011 for 6 Mbps, last two bit value = 3 */
        /* R4-R1 : 1111 for 9 Mbps, last two bit value = 3 */
        /* R4-R1 : 1010 for 12 Mbps, last two bit value = 2 */
        /* R4-R1 : 1110 for 18 Mbps, last two bit value = 2 */
        /* R4-R1 : 1001 for 24 Mbps, last two bit value = 1 */
        /* R4-R1 : 1101 for 36 Mbps, last two bit value = 1 */
        /* R4-R1 : 1000 for 48 Mbps, last two bit value = 0 */
        /* R4-R1 : 1100 for 54 Mbps, last two bit value = 0 */
        else /* for ofdm case, cck case */
        {
            /* for ofdm rates */
            if (lmac_rate & UU_OFDM_RATE_FLAG)
            {
                if(((lmac_rate & UU_OFDM_MODULATION_TYPE_IND) == UU_OFDM_BPSK) && (l_index <= UU_RATE_9M_BIT))
                {
                    basic_rate = UU_HW_RATE_6M;
                }
                else if(((lmac_rate & UU_OFDM_MODULATION_TYPE_IND) == UU_OFDM_QPSK) && (l_index <= UU_RATE_18M_BIT))
                {
                    basic_rate = UU_HW_RATE_12M;
                }
                else
                {
                    basic_rate = UU_HW_RATE_24M;
                }
            }
            /* for CCK rates */
            else if (l_index < UU_SUPPORTED_CCK_RATE )
            {
                basic_rate = lmac_rate;
            }
            else
            {
                basic_rate = UU_HW_RATE_1M;
            }
        }
    }
    return basic_rate;
} /* uu_assign_basic_rate */


/* This function returns number of spatial streams as per mcs index in HT case. */
static uu_int32 uu_mcs_to_nss (uu_uint8 rate) /* This is for HT. This returns nss as per mcs index. */
{
    if(rate < UU_MAX_MCS_N_SUP)
    {
        return (((rate >> UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & UU_VHT_NSS_FROM_MCS_MASK) + 1);
    }
    else
    {
        return 1; /* 1 spatial stream */
    }
}


/* This function returns data rate in 100 kbps as per 802.11n REV_mb D12 section 20.6 with respect to channel used and sgi support. */
static uu_uint64 uu_mcs_to_rate(uu_uint8 mcs, int chan_type, bool sgi)
{
    uu_uint64 result_rate;

    if (chan_type == CBW40)
    {
        if (sgi)
        {
            result_rate = uu_ht_mcs_table_g[mcs].mcs_rate_40_sgi;
        }
        else
        {
            result_rate = uu_ht_mcs_table_g[mcs].mcs_rate_40;
        }
    }
    else
    {
        if (sgi)
        {
            result_rate = uu_ht_mcs_table_g[mcs].mcs_rate_20_sgi;
        }
        else
        {
            result_rate = uu_ht_mcs_table_g[mcs].mcs_rate_20;
        }
    }
    return result_rate;
} /* uu_mcs_to_rate */

/* This function returns data rate in 250 kbps as per 802.11ac D3.0 section 22.5 with respect to channel used and sgi support and as per number of streams. */
static uu_uint64 uu_mcsac_to_rate(uu_uint8 mcsac, uu_uint8 n_sts, int chan_type, bool sgi)
{
    uu_uint64 result_rate;

    if (chan_type == CBW160 || chan_type == CBW80_80)
    {
        if(sgi)
        {
            /* for channel bandwidth 160 or 80+80 with sgi for a mcs index */
            result_rate = uu_vht_mcs_table_g[(n_sts * UU_VHT_MCS_INDEX_MAXIMUM) + mcsac].mcs_ac_rate_160_sgi;
        }
        else
        {
            /* for channel bandwidth 160 or 80+80 with no sgi for a mcs index */
            result_rate = uu_vht_mcs_table_g[(n_sts * UU_VHT_MCS_INDEX_MAXIMUM) + mcsac].mcs_ac_rate_160;
        }
    }

    else if (chan_type == CBW80)
    {
        if(sgi)
        {
            /* for channel bandwidth 80 with sgi for a mcs index */
            result_rate = uu_vht_mcs_table_g[(n_sts * UU_VHT_MCS_INDEX_MAXIMUM) + mcsac].mcs_ac_rate_80_sgi;
        }
        else
        {
            /* for channel bandwidth 80 with no sgi for a mcs index */
            result_rate = uu_vht_mcs_table_g[(n_sts * UU_VHT_MCS_INDEX_MAXIMUM) + mcsac].mcs_ac_rate_80;
        }
    }


    else if (chan_type == CBW40)
    {
        if(sgi)
        {
            /* for channel bandwidth 40 with sgi for a mcs index */
            result_rate = uu_vht_mcs_table_g[(n_sts * UU_VHT_MCS_INDEX_MAXIMUM) + mcsac].mcs_ac_rate_40_sgi;
        }
        else
        {
            /* for channel bandwidth 40 with no sgi for a mcs index */
            result_rate = uu_vht_mcs_table_g[(n_sts * UU_VHT_MCS_INDEX_MAXIMUM) + mcsac].mcs_ac_rate_40;
        }
    }

    else
    {
        if(sgi)
        {
            /* for channel bandwidth 20 with sgi for a mcs index */
            result_rate = uu_vht_mcs_table_g[(n_sts * UU_VHT_MCS_INDEX_MAXIMUM) + mcsac].mcs_ac_rate_20_sgi;
        }
        else
        {
            /* for channel bandwidth 20 with no sgi for a mcs index */
            result_rate = uu_vht_mcs_table_g[(n_sts * UU_VHT_MCS_INDEX_MAXIMUM) + mcsac].mcs_ac_rate_20;
        }
    }
    return result_rate;
} /* uu_mcsac_to_rate */


#ifdef UU_WLAN_RC_RTL_TABLE
/*
 * To avoid the division operation
 * Multiplying the (1/rate_250kbps) with Highest 40 bit value (0xFFFFFFFFFF)
 *
 * Calculating the n_syms = multiflyer * (phy service bits + frame len in bits + phy tail bits)
 *
 * Later Dividing the  n_syms with 0xFFFFFFFFFF to get actual number of symbols
 */

/* Returns number of symbols. Input: MAC frame length in bytes, STBC option (0/1/2), and Rate : (0xFFFFFFFFFF /(1/rate_250kbps)) 40 bits in len */
static uu_uint16 get_nsyms (uu_uint32 mac_fr_len, int stbc, uu_uint64 rate)
{
    uu_uint64 n_syms;
    uu_char add = 0;

    /*
     * Here rate equivalents to (0xFFFFFFFFFF/rate_250kbps)
     */

    n_syms = rate * (PHY_SERVICE_NBITS + 8 * mac_fr_len + PHY_TAIL_NBITS);

    /* Ceiling is required or not */
    if ((n_syms>>8) & 0xffffffff)
    {
       add = 1;
    }

    /* Dividing scaler multiplication output with 0xFFFFFFFFFF */
    n_syms = (n_syms >> UU_ONE_BY_RATE_RESOLUTION);

    /* Ceiling to next value */
    n_syms += add;

    /* If stbc is true, then number of symbols should be even */
    if (stbc)
    {
        /* If number of symbols is odd, then add 1. */
        if (n_syms & 0x01)
        {
            n_syms++;
        }
    }

    return (uu_uint16)n_syms;
}
#else
/* Returns number of symbols. Input: MAC frame length in bytes, STBC option (0/1/2), and Rate : HT MCS index, VHT MCS index, Legacy rates */
static int get_nsyms (int mac_fr_len, int stbc, int rate)
{
    int n_syms;

    /*
     * For legacy case: 1 Mbps : rate = 10 * 4 = 40
     * For mcs : 6.5 Mbps : rate = 65 * 4 = 260
     */

    /* If stbc is enabled */
    if (stbc)
    {
        /* STBC needs to have even number of symbols */
        n_syms = DIV_CEIL_EVEN((PHY_SERVICE_NBITS + 8 * mac_fr_len + PHY_TAIL_NBITS), rate);
    }
    else /* If stbc is disabled */
    {
        n_syms = DIV_CEIL((PHY_SERVICE_NBITS + 8 * mac_fr_len + PHY_TAIL_NBITS), rate);
    }

    return n_syms;
} /* get_nsyms */

#endif

/** This function updates the duration for HT mixed mode and green field common parts
 * and the reamining 8 us is filled in uu_calc_frame_duration for HT-mixed mode case.
 * This part is common for HT-GF and HT-MF formats, excluding legacy signals. TODO
 * Return UU_FAILURE if fails or duration in success, 32 bits in length
 * @param[in] txvec of the packet to be transmitted, 32 bytes in length.
 */
/* uu_uint16 uu_calc_ht_frame_duration (int mac_len, uu_uint8 rate, int chan_type, bool sgi, uu_uint8 stbc, uu-uint8 n_ess) */
static uu_int16 uu_calc_ht_frame_duration (uu_wlan_tx_vector_t *txvec)
{
    uu_uint16 duration;
    uu_uint8 n_sts, nss;
    int htdltf, hteltf, htltf;
    uu_uint64 mcs_250kbps;
    int nsyms;

    /* Nss = Number of spatial streams. Get from mcs. Values: 1 / 2 / 3 / 4. */
    /* 802.11n REV_mb D12, Section 20.3.9.4.6, refer to tables */
    nss = uu_mcs_to_nss(txvec->mcs);

    /* Ness = Number of extension spatial streams (only for HT case). Values: 0 / 1 / 2 / 3
     * stbc = Number of stbc in HT case. It cant be greater than Nss. Values: 0 / 1 / 2
     * Nsts = Number of space time streams. min(Nss+stbc, 4). Values: 1 / 2 / 3 / 4
     * Nhtdltf = Number of data HT long training field. (1 for sts = 1, 2 for sts = 2, 4 for sts = 3 or 4) Values: 1 / 2 / 3 / 4
     * Nhteltf = Number of extension HT long training field. (0 for Ness = 0, 1 for Ness = 1, 2 for Ness = 2, 4 for Ness = 3) Values: 0 / 1 / 2 / 3
     * Nhtltf = Total number of HT long training field. Values: 1 / 2 / 3 / 4 / 5
     * Nhtltf = min (Nhtdltf + Nhteltf, 5)
     */

    /*
       Number of Spatial Streams (from MCS)     STBC field     Number of space-time streams
       1             0         1
       1             1         2
       2             0         2
       2             1         3
       2             2         4
       3             0         3
       3             1         4
       4             0         4
     */

    /* As per the specs, stbc can't have more value than nss. Table 20-12 in 802.11REVmb D12 */
    if (txvec->stbc > nss)
    {
        return 0; /* NOTE: Not expected this ERROR. Returning the best possible here */
    }

    /* Nsts should not be greater than Maximum streams supported in HT */
    if ((nss + txvec->stbc) > UU_MAX_STREAM_HT)
    {
        return 0; /* NOTE: Not expected this ERROR. Returning the best possible here */
    }

    n_sts = nss + txvec->stbc;

    /*
       Nsts        Nhtdltf
       1         1
       2         2
       3         4
       4         4
     */

    htdltf = (n_sts == 3) ? 4 : n_sts;

    /*
       Ness      Nhteltf
       0         0
       1         1
       2         2
       3         4
     */
    hteltf = (txvec->num_ext_ss == 3) ? 4 : txvec->num_ext_ss;

    htltf = htdltf + hteltf;

    /* htltf <= 5 Ref : 20.3.9.4.6 */
    if (htltf > 5)
    {
        return 0; /* NOTE: Not expected this ERROR. Returning the best possible here */
    }

    /* duration (MF) = L-STF(8 us) + L-LTF(8 us) + LSIG(4 us) + HT-SIG(8 us) +
       HTSTF(4 us) + HTLTF * (Data HT-LTFs 4us per LTF + Extension HT-LTFs 4us per LTF) */

    /* duration (GF) = HT-GF-STF(8 us) + HT-LTF1(8 us) + HT-SIG(8 us) + HTLTF * (Data HT-LTFs 4us per LTF + Extension HT-LTFs 4us per LTF) */
    /* So we add as duration (common) = 16 us + 8 us + HTLTF * (Data HT-LTFs 4us per LTF + Extension HT-LTFs 4us per LTF) */
    duration = PHY_PREAMBLE_TIME + PHY_HT_SIG + (htltf * PHY_PER_LTF);

    /* duration for data */
    /* For 6.5 Mbps, we will receive mcs_250kbps as 26 */
    mcs_250kbps = uu_mcs_to_rate(txvec->mcs, txvec->ch_bndwdth, txvec->is_short_GI);

    /* Number of symbols */
    /* Now mcs_kbps is needed to divide by 10, so that we can get exact rate in Mbps */
    /* Then we multiple the rate by 4 for the duration calculation as per physical layer */
    nsyms = get_nsyms (txvec->ht_length, txvec->stbc, mcs_250kbps);
    /* duration += 4 us * symbols */
    duration += PHY_SYMBOL_TIME * nsyms;
    /* This is 0 for 5 GHz and 6 for 2.4 GHz */
    duration += dot11_signal_extension;

    return duration;
} /* uu_calc_ht_frame_duration */


#if 0 /* Not required, as it is implemented in PHY */
uu_uint16 uu_calc_htmf_l_length (uu_wlan_tx_vector_t *txvec)
{
    /*Getting the txtime as per shown in 20.3.11.1 and 20.4.3 */
    /* We don't consider txtime case in HT-GF */

    /* For HT-MF */
    /* TXTIME = Tl_stf + Tl_ltf + Tl_sig + Tht_preamble + Tht_sig + Tsym * CEIL[(Tsyms * Nsym) / Tsym] + SignalExtension */

    /* Initializing ret_length with 0 */
    uu_uint16 ret_len = 0;
    int txtime;
    txtime = uu_calc_ht_frame_duration (txvec);
    /*Since this is HT-MF frame */
    txtime = PHY_SIGNAL_TIME + PHY_HT_STF;

    /* As per 9.23.4 Calculation of L-length and L_datarate */
    /*ret_len = ((((txtime - dot11_signal_extension) - 20) / 4) * 3) - 3; */
    ret_len = ((((txtime - dot11_signal_extension) - 20) >> 2) * 3) - 3;

    return ret_len;
} /* uu_calc_htmf_l_length */
#endif /* #if 0 */


static uu_uint16 get_cck_duration (uu_uint32 mac_fr_len, uu_uint16 is_long_pre, uu_uint64 rate)
{
    uu_uint64 duration;
    uu_char add = 0;

    /*
     * Here rate equivalents to (0xFFFFFFFFFF/rate_250kbps)
     */

    /* Refer to: section 17.3.4 in P802.11 REV_mb D12 */
    /* DURATION = PreambleLength + PLCPHeaderTime + Ceiling(((LENGTH+PBCC) × 8) / DATARATE) */
    /* Since rate is in terms of 250 kbps, we multiply the length with 4 */
    duration = rate * (mac_fr_len * 8 * 4);

    /* Ceiling is required or not */
    if ((duration>>8) & 0xffffffff)
    {
       add = 1;
    }

    /* Dividing scaler multiplication output with 0xFFFFFFFFFF */
    duration = (duration >> UU_ONE_BY_RATE_RESOLUTION);

    /* Ceiling to next value */
    duration += add;

    if (is_long_pre)
        duration += PHY_PLCP_TIME;
    else
        duration += PHY_PLCP_SHORT_TIME;

    return (uu_uint16)duration;

}


/* This is to calculate the frame duration considering Preamble headers */
/* uu_uint16 uu_calc_frame_duration (int mac_len, uu_uint8 rate, uu_uint8 is_long_preamble, int chan_type, bool sgi, uu_wlan_frame_format_t format, uu_uint8 stbc, uu_uint8 n_ess) */
uu_uint16 uu_calc_frame_duration (uu_wlan_tx_vector_t *txvec)
{
    uu_uint16 duration;
    int n_sts; /* NOTE : n_sts will be n_tx for tx_vector */
    uu_uint32 nsyms;
    uu_uint64 rate_250kbps;
    uu_uint64 mcs_250kbps;
    int vhtltf;

    /* For 11ac */
    if (txvec->format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        /* 802.11ac D3.0 Section 22.3.8.2.5, Refer to tables */

        /* NSTS,    total NVHTLTF
           1     1
           2     2
           3     4
           4     4
           5     6
           6     6
           7     8
           8     8
         */

        n_sts = txvec->tx_vector_user_params[0].num_sts + 1;

        /* vhtltf depends on the above table as n_sts */
        vhtltf = ((n_sts == 3) || (n_sts == 5) || (n_sts == 7)) ? (n_sts + 1) : n_sts;

        /* L-STF = 8 us
           L-LTF = 8 us
           L-SIG = 4 us
           VHT-SIG-A = 8 us
           VHT-STF = 4 us
           VHT-SIG-B = 4 us
           VHT-LTF = 4 us per vhtltf symbol

           So, duration = 16 + 4 + 8 + 4 + 4 * vhtltf + 4 */

        duration = PHY_PREAMBLE_TIME + PHY_SIGNAL_TIME + PHY_VHT_SIG_A + PHY_VHT_STF + (vhtltf * PHY_PER_LTF) + PHY_VHT_SIG_B;
        /* mcs_kbps = kbps * 4 */
        mcs_250kbps = uu_mcsac_to_rate(txvec->tx_vector_user_params[0].vht_mcs, txvec->tx_vector_user_params[0].num_sts,
                                        txvec->ch_bndwdth, txvec->is_short_GI);

        /* Number of symbols */
        nsyms = get_nsyms (txvec->tx_vector_user_params[UU_VHT_PHY_USR].apep_length, txvec->stbc, mcs_250kbps);
        /* 4 us per symbol */
        duration += PHY_SYMBOL_TIME * nsyms;
    }
    else if ((txvec->format == UU_WLAN_FRAME_FORMAT_HT_MF) || (txvec->format == UU_WLAN_FRAME_FORMAT_HT_GF)) /* For 11n */
    {
        /* First fill the duration which is common for both mixed mode and greenfield */
        duration = uu_calc_ht_frame_duration (txvec);
        /* duration (MF) += 4 us + 4 us */
        if (txvec->format == UU_WLAN_FRAME_FORMAT_HT_MF)
        {
            duration += (PHY_SIGNAL_TIME + PHY_HT_STF);
        }
    }
    /* for legacy case */
    else
    {
        /* No stbc case present in non-HT case */
        txvec->stbc = 0;

        /* for OFDM */
        if (txvec->L_datarate & UU_OFDM_RATE_FLAG)
        {
            rate_250kbps = uu_lrate_table_g[(uu_legacy_ofdm_rate_to_index_map_g[(txvec->L_datarate & UU_OFDM_HW_MASK_INDEX)])].kbps250;
            /* duration = 16 us + 4 us + data time + ofdm signal extension */
            duration = PHY_PREAMBLE_TIME + PHY_SIGNAL_TIME;
            /* for 1 mbps, rate_250kbps has value 4 */
            /* Now rate is 4 times of the exact rate in Mbps */
            /* For the duration calculation as per physical layer */
            nsyms = get_nsyms (txvec->L_length, txvec->stbc, rate_250kbps);
            duration += PHY_SYMBOL_TIME * nsyms;
            duration += dot11_signal_extension;
        }
        else /* for CCK */
        {
            rate_250kbps = uu_lrate_table_g[lmac_rate_to_index(txvec->L_datarate)].kbps250;
            /* duration = preamble (long or short) + data time */
            duration = get_cck_duration (txvec->L_length, txvec->is_long_preamble, rate_250kbps);
        }
    }

    return duration;
} /* uu_calc_frame_duration */


/* EOF */

