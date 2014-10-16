/*******************************************************************************
 **                                                                            **
 ** File name :  uu_wlan_test_init.c                                                **
 **                                                                            **
 ** Copyright © 2010, Uurmi Systems                                            **
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

/** 
 * @uu_wlan_test_init.c
 * Contains main Module hooks like module init and module exit.
 */

#include "uu_datatypes.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_errno.h" 
#include "uu_wlan_reg.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_cp_ctl_frame_gen.h"
#include "uu_netlink.h"
#include "uu_wlan_main.h"
#include "uu_wlan_mac_stub.h"

MODULE_AUTHOR("UURMI Systems");
MODULE_DESCRIPTION("UMAC simulation for testing LMAC");
MODULE_SUPPORTED_DEVICE("uurmi 802.11ac WLAN cards");
MODULE_LICENSE("Dual BSD/GPL");

uu_kern_netlink_t k_nl_g; // Netlink Socket for receiving data from port 24
uu_kern_netlink_t k_nl_g_t; // Netlink Socket for receiving registers from port 25

/*  uu_umac_stub_nl_send_frame
 *Receives data from LMAC and post the data to testdriver netlink socket
 */
int  uu_umac_stub_nl_send_frame(unsigned char *frame, int len)
{
    uu_uint32 i=0;

    if (k_nl_g.nl_sk == NULL)
    {
        return -1; // Nl sock create failed
    }

    for(i=0;i<len;i++)
        printk("%x ",frame[i]);
    printk("\n");

    if ((UU_WLAN_GET_FC0_VALUE(frame) & IEEE80211_FC0_TYPE_MASK)  == IEEE80211_FC0_TYPE_CTL)
    {
        return 0;
    }

    send_to_groups(k_nl_g.nl_sk, 0, UU_NL_PROTO_UMAC_STUB, frame, len);
    return 0;
} /* uu_umac_stub_nl_send_frame*/


/* nl_callbck 
 *Receives data from testdriver netlink socket
 *The data posted to test driver netlink socket is transmitted from user space applications
 */
uu_void nl_callbck (struct sk_buff *skb)
{
    uu_int32 len;
    struct nlmsghdr *nlh = NULL;
    uu_uchar *data = NULL;

    nlh = (struct nlmsghdr *)skb->data;
    len = nlh->nlmsg_len - NLMSG_LENGTH(0);
    data = NLMSG_DATA( nlh );

    uu_umac_stub_indication(data, len);
    return;
}/* nl_callbck */ 

/* set_config_registers */
uu_void set_config_registers(struct sk_buff *skb)
{
    uu_int32 len;
    struct nlmsghdr *nlh = NULL;
    uu_uchar *data = NULL;

    nlh = (struct nlmsghdr *)skb->data;
    len = nlh->nlmsg_len - NLMSG_LENGTH(0);
    data = NLMSG_DATA( nlh );

    uu_umac_config_registers(data,len);
    return 0; 
}

/* ADD Station */
static int uu_add_station(void)
{
    uu_uchar addr[6];
    uu_wlan_lmac_config_t cmd_info;
    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_ASSO_ADD;
    /** Filling of station info */
    cmd_info.cmd_data.asso_add.aid = 1;
    cmd_info.cmd_data.asso_add.ch_bndwdth = CBW40;
    addr[0]=0xfc;
    addr[1]=0xf8;
    addr[2]=0xb0;
    addr[3]=0x10;
    addr[4]=0x20;
    addr[5]=0x04;
    memcpy(cmd_info.cmd_data.asso_add.addr, addr, IEEE80211_MAC_ADDR_LEN);
    /* TODO: Fill the remaining part of the structure */
    uu_wlan_lmac_config(&cmd_info);
    return 0;
}

static uu_int32 uu_wlan_del_station(void)
{
    uu_uchar addr[6];
    uu_wlan_lmac_config_t cmd_info;
    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_ASSO_DEL;
    cmd_info.cmd_data.asso_add.ch_bndwdth = CBW40;
    addr[0]=0xfc;
    addr[1]=0xf8;
    addr[2]=0xb0;
    addr[3]=0x10;
    addr[4]=0x20;
    addr[5]=0x04;
    /** Filling of station info */
    cmd_info.cmd_data.asso_del.aid = 1;
    memcpy(cmd_info.cmd_data.asso_del.addr, addr, IEEE80211_MAC_ADDR_LEN);
    /* TODO: Is this required, for deleting station context? */
    uu_wlan_lmac_config(&cmd_info);
    /** Filling associated station information */
    return 0;
}

/* Add BA session */
static int uu_add_ba_session(void)
{
    uu_uchar addr[6];
    uu_wlan_lmac_config_t cmd_info;
    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_BA_SESS_ADD;
    addr[0]=0xfc;
    addr[1]=0xf8;
    addr[2]=0xb0;
    addr[3]=0x10;
    addr[4]=0x20;
    addr[5]=0x04;
    /** Filling of ADDBA info */
    memcpy(cmd_info.cmd_data.addba.addr, addr, IEEE80211_MAC_ADDR_LEN);
    cmd_info.cmd_data.addba.tid = 0;
    cmd_info.cmd_data.addba.is_dir_tx = 0;
    cmd_info.cmd_data.addba.ssn = 1;
    cmd_info.cmd_data.addba.type=1;
    cmd_info.cmd_data.addba.bufsz=8;
    uu_wlan_lmac_config(&cmd_info);
    return 0;
}

static int uu_del_ba_session(void)
{
    uu_uchar addr[6];
    uu_wlan_lmac_config_t cmd_info;
    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_BA_SESS_DEL;
    addr[0]=0xfc;
    addr[1]=0xf8;
    addr[2]=0xb0;
    addr[3]=0x10;
    addr[4]=0x20;
    addr[5]=0x04;
    /** Filling of DELBA info */
    memcpy(cmd_info.cmd_data.delba.addr, addr, IEEE80211_MAC_ADDR_LEN);
    cmd_info.cmd_data.delba.tid = 0;
    cmd_info.cmd_data.delba.is_dir_tx = 0;
    UU_WLAN_LOG_DEBUG(("UMAC-DELBA:  STA Addr: %x:%x:%x:%x:%x:%x, TID:%d \n",
                cmd_info.cmd_data.delba.addr[0], cmd_info.cmd_data.delba.addr[1], cmd_info.cmd_data.delba.addr[2], cmd_info.cmd_data.delba.addr[3],
                cmd_info.cmd_data.delba.addr[4], cmd_info.cmd_data.delba.addr[5], cmd_info.cmd_data.delba.tid));
    uu_wlan_lmac_config(&cmd_info);
    return 0;
}

/*  Module initialization 
 *Creates testdriver netlink socket with protoid of UU_NL_PROTO_UMAC_STUB
 */

uu_lmac_ops_t lmac_ops_g=
{
    .umacCBK = uu_wlan_umac_cbk
};

static uu_int32 __init uu_umac_stub_nl_init(uu_void)
{
    uu_add_station();
    uu_add_ba_session();

    k_nl_g.MAX_PAYLOAD = UU_MAX_PAYLOAD;
    k_nl_g_t.MAX_PAYLOAD = UU_MAX_PAYLOAD;

    // Netlink Socket creation for Test driver
    k_nl_g.nl_sk = k_create_nl (&init_net, UU_NL_PROTO_UMAC_STUB, 0,
            nl_callbck, NULL, THIS_MODULE);

    if (k_nl_g.nl_sk == NULL)
    {
        printk("UMAC-STUB :: Nl sock create failed");
        return 0;
    }

    printk(KERN_INFO "UMAC:Netlink Socket created for Sending Frames\n");

    // Netlink Socket creation for UMAC-LMAC interface registers
    k_nl_g_t.nl_sk = k_create_nl (&init_net, UU_NL_CONFIG_REGISTERS, 0,
            set_config_registers, NULL, THIS_MODULE);

    if (k_nl_g_t.nl_sk == NULL)
    {
        printk("CONFIG :: Nl sock create failed");
        return 0;
    }
    printk(KERN_INFO "UMAC:Netlink Socket created for initializing registers \n");

    uu_lmac_reg_ops(&lmac_ops_g);
    UU_REG_LMAC_FILTER_FLAG = UU_WLAN_FILTR_NO_FILTER; /* Filters Control Frames */
    return 0;
} /*uu_umac_stub_nl_init */
module_init(uu_umac_stub_nl_init);


/* Module exit function
 *Releases testdriver netlink socket
 */
static uu_void __exit uu_umac_stub_nl_exit(uu_void)
{
    UU_WLAN_LOG_DEBUG((" Driver unloaded\n"));

    uu_wlan_del_station();
    uu_del_ba_session();

    /* LMAC termination*/
    uu_wlan_lmac_shutdown();
    uu_wlan_lmac_stop();

    k_nl_release(k_nl_g.nl_sk);
    k_nl_release(k_nl_g_t.nl_sk);
} /* uu_umac_stub_nl_exit */
module_exit(uu_umac_stub_nl_exit);

/* EOF */


