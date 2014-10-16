#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/kthread.h>
#include <linux/version.h>

#include <net/sock.h>
#include <net/netlink.h>

#include "uu_wlan_fwk_log.h" 

#include "uu_netlink.h"

#include "uu_phy_stub_nl.h"
#include "uu_wlan_phy_stub.h" 

static uu_kern_netlink_t k_nl_g;

#define PHY_CMD 2

phy_stub_proto_ops_t *phy_ops_gp;

int uu_phy_stub_nl_reg_ops(phy_stub_proto_ops_t *ops)
{
    if (ops != NULL)
        phy_ops_gp = ops;
    return 0;
}
EXPORT_SYMBOL_GPL(uu_phy_stub_nl_reg_ops);


int uu_phy_stub_nl_dereg_ops(void)
{
    phy_ops_gp = NULL;
    return 0;
}
EXPORT_SYMBOL_GPL(uu_phy_stub_nl_dereg_ops);


int  uu_phy_stub_nl_send_frame(unsigned char *frame, int len)
{
    if (k_nl_g.nl_sk == NULL)
    {
        UU_WLAN_LOG_ERROR(("Nl sock create failed"));
        return -1;
    }
    send_to_groups(k_nl_g.nl_sk, 0, UU_NL_PROTO_PHY_STUB, frame, len);
    return 0;
}
EXPORT_SYMBOL_GPL(uu_phy_stub_nl_send_frame);


void nl_callback (struct sk_buff *skb)
{
    int                len;
    struct nlmsghdr    *nlh = NULL;
    unsigned char      *data = NULL;
    int cmd_type;
    nlh = (struct nlmsghdr *)skb->data;
    len = nlh->nlmsg_len - NLMSG_LENGTH(0);
    data = NLMSG_DATA( nlh );

    if(data == NULL)
    {
        return;
    }

#ifndef UU_WLAN_END_TO_END_TESTING

    cmd_type = data[0]-0x30;
    UU_WLAN_LOG_DEBUG(("SEND-CMD / PHY-CMD %d\n",cmd_type));

    if(cmd_type == PHY_CMD)
    {
        uu_phy_stub_indication(data+1, len-1);
    }

    else
    {
        uu_phy_stub_proc_nl_rx_frame(data+1, len-1);
    }

    return;

#else
    uu_phy_stub_start_rx_proc(data, len);
    return;

#endif
}

static int __init uu_phy_stub_nl_init(void)
{
    k_nl_g.MAX_PAYLOAD = UU_MAX_PAYLOAD;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,6,0)
    k_nl_g.nl_sk = k_create_nl (&init_net, UU_NL_PROTO_PHY_STUB, 0,
            nl_callback, NULL, THIS_MODULE);
#else
       struct netlink_kernel_cfg cfg = {
                .input  = nl_callback,
                .groups = 0
        };

        k_nl_g.nl_sk = netlink_kernel_create (&init_net, UU_NL_PROTO_PHY_STUB, &cfg);
#endif
    if (k_nl_g.nl_sk == NULL)
    {
        UU_WLAN_LOG_ERROR(("Nl sock create failed"));
        return -1;
    }
    UU_WLAN_LOG_DEBUG((KERN_INFO "Netlink Socket created for Sending Frames\n"));
    return 0;
}


static void __exit uu_phy_stub_nl_exit(void)
{
    UU_WLAN_LOG_DEBUG((KERN_INFO "exiting phy_stub module\n"));
    k_nl_release(k_nl_g.nl_sk);
}


module_init(uu_phy_stub_nl_init);
module_exit(uu_phy_stub_nl_exit);


/* EOF */


