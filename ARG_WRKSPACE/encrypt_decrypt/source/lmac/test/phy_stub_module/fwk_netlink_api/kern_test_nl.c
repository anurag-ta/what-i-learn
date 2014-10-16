#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#include <linux/kthread.h>

#include "uu_netlink.h"

uu_kern_netlink_t k_nl_g;
struct task_struct *th_mcast;

int multicast (void *ptr)
{
	int msglen = 10;
	char msg[10] = "hello";
	
	while (!kthread_should_stop())
	 {
		// BROADCAST TODO test with groups and user_space
		send_to_groups(k_nl_g.nl_sk, 0, NETLINK_BROADCAST, msg, msglen);
	 }
	send_to_groups (k_nl_g.nl_sk, NLMSG_DONE, NETLINK_BROADCAST, msg, msglen);
	return 0;
}

void unicast (struct sk_buff *skb)
{
	int pid;
	int ret;
	char msg[20] = "hey\n";
	int msglen = strlen (msg);
	pid = get_pid_from_skb(skb);
	ret = send_unicast (k_nl_g.nl_sk, pid, msg, msglen);
}

static int __init hello_init(void)
{
    printk("Entering: %s\n",__FUNCTION__);


	/* Either unicast or multicast must be used */

	/* unicast */
//	k_nl_g.nl_sk = k_create_nl (&init_net, UU_NETLINK_PROTO, 0,
//							unicast, NULL, THIS_MODULE);
	/* EOF UNICAST */

	/* Multicast */
	k_nl_g.nl_sk = k_create_nl (&init_net, UU_NETLINK_PROTO, 0,
					 NULL, NULL, THIS_MODULE);
	k_nl_g.MAX_PAYLOAD = UU_MAX_PAYLOAD;
	th_mcast = kthread_run (multicast, NULL, "send-msg");
	if(!k_nl_g.nl_sk)
	{
        	printk(KERN_ALERT "Error creating socket.\n");
	        return -10;
 	}
	/* EOF MULTICAST */
	msleep (300);
    return 0;
}

static void __exit hello_exit(void){
    printk(KERN_INFO "exiting hello module\n");
//	kthread_stop(th_mcast);
    k_nl_release(k_nl_g.nl_sk);
}

module_init(hello_init);
module_exit(hello_exit);

/* EOF */

