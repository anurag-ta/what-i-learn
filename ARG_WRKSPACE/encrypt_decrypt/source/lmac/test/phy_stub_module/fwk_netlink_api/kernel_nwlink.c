//#include <net/sock.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/version.h>

#include "uu_return_types.h"
#include "uu_netlink.h"

#define UU_PRINT(msg) printk(KERN_INFO msg)

/* Create netlink sock in kernel */
struct sock *
k_create_nl (struct net *net, int unit, unsigned int groups,
	void (*input)(struct sk_buff *skb), struct mutex *cb_mutex,
	struct module *module)
{
	struct sock *sk;
	UU_PRINT( "k_create netlink\n");
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,6,0)
	sk = netlink_kernel_create (net, unit, groups, input, 
			cb_mutex, module);
#endif
	return sk;
}

/* Send message to groups */
int 
send_to_groups (struct sock *nl_sk, unsigned int flag,
	unsigned groups, unsigned char *msg, int msglen)
{
	int ret;
	struct sk_buff *rskb = NULL;
	struct nlmsghdr *nlh;
	
	if ((msg == NULL) || msglen <= 0)
	 {
		return UU_INVALVAL;
	 }
	if (msglen > UU_MAX_PAYLOAD)
	 {
		return UU_FAILURE;
		// TODO Truncate the message upt max_payload
		// now only else
		// while strcpy
		// But NULL has to be added at the end!
	 }
   	rskb = nlmsg_new(msglen,0);
   	if ( !rskb )
   	{
      		//UU_KERNEL_LOG_ERROR(("AFDX RECV: Alloc_skb failed\n"));
		return UU_FAILURE;
   	}
   	nlh = nlmsg_put(rskb, 0, 0, NLMSG_DONE, msglen, 0);

   	memset(nlh, 0, sizeof(struct nlmsghdr));
	//nlh->nlmsg_len = NLMSG_SPACE (msglen); // change this also t    o msglen???
	nlh->nlmsg_len = NLMSG_LENGTH (msglen); // change this also t    o msglen???
	nlh->nlmsg_pid = 0;
	nlh->nlmsg_flags = flag;
	
	memcpy (NLMSG_DATA(nlh), msg, msglen);
	
	NETLINK_CB(rskb).dst_group = groups;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,6,0)
	NETLINK_CB(rskb).pid = 0;
#endif

	ret = netlink_broadcast (nl_sk, rskb, 0, 1, GFP_KERNEL);

	if (ret != 0)
	 {
		//TODO what to do here???
		return UU_FAILURE;
	 }
	return UU_SUCCESS; //TODO return anything else??
}


/* Unicast msg */
int 
send_unicast (struct sock *nl_sk, int pid, 
	unsigned char *msg, int msglen)
{
	struct sk_buff *rskb;
	struct nlmsghdr *nlh;
	
	int ret;

	if (msglen > UU_MAX_PAYLOAD)
	 {
		return UU_FAILURE;
	 }

   	rskb = nlmsg_new(msglen,0);
   	if ( !rskb )
   	{
      		//UU_KERNEL_LOG_ERROR(("AFDX RECV: Alloc_skb failed\n"));
		return UU_FAILURE;
   	}
   	nlh = nlmsg_put(rskb, 0, 0, NLMSG_DONE, msglen, 0);

   	memset(nlh, 0, sizeof(struct nlmsghdr));
	//nlh->nlmsg_len = NLMSG_SPACE (msglen); // change this also t    o msglen???
	nlh->nlmsg_len = NLMSG_LENGTH (msglen); // change this also t    o msglen???
	nlh->nlmsg_pid = 0;
	nlh->nlmsg_flags = NLMSG_DONE;

	NETLINK_CB (rskb).dst_group = 0; //UNICAST

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,6,0)
	NETLINK_CB (rskb).pid = pid;
#endif
	
	memcpy (nlmsg_data (nlh), msg, msglen);
	
	ret = nlmsg_unicast (nl_sk, rskb, pid);
	
	if (ret < 0)
	 {
		UU_PRINT ("Err while sending to user\n");
		return UU_FAILURE;
	 }
	return msglen;
}


/* 
 * Recieve msg from sender.
 * Return pid from the nlmsghdr field
 */
int get_pid_from_skb (struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	
	nlh = (struct nlmsghdr *)skb->data;
	return nlh->nlmsg_pid;
}


/* Netlink Kernel Release */
void k_nl_release (struct sock *nl_sk)
{
	return netlink_kernel_release (nl_sk);
}


/* EOF */

