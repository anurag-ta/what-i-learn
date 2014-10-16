#ifndef __UU_NETLINK__
#define __UU_NETLINK__


#define UU_MAX_PAYLOAD 5120

/* UURMI defined netlink protocols */
#define UU_NL_PROTO_LOG 	31
#define UU_NL_PROTO_IPRECV	28
#define UU_NL_PROTO_UMAC_STUB  24
#define UU_NL_CONFIG_REGISTERS   25 
#define UU_NL_PROTO_PHY_STUB   29 


/* TODO: Relook ???? */
#define NETLINK_BROADCAST	31 

#ifdef __KERNEL__
#include <linux/netlink.h>
typedef struct uu_kern_netlink
{
	int LOG_NETLINK_PROTO;
	struct sock *nl_sk;
	unsigned int groups;
	int MAX_PAYLOAD;
	
} uu_kern_netlink_t;

extern struct sock * k_create_nl (struct net * /*net*/, int /*unit*/,
					unsigned int /*groups*/,
					void (*input)(struct sk_buff * /*skb*/),
					struct mutex * /*cb_mutex*/,
					struct module * /*module*/);

extern void k_nl_release (struct sock *nl_sk);

extern int send_to_groups (struct sock *nl_sk, unsigned int flag, 
				unsigned groups, unsigned char *msg, int msglen);

extern int send_unicast (struct sock *nl_sk, int pid, 
				unsigned char *msg, int msglen);


extern int get_pid_from_skb (struct sk_buff *skb);

#endif


/* Netlink @ user space */

#ifndef __KERNEL__
typedef struct uu_user_netlink
{
	int LOG_NETLINK_PROTO;
	int sock_fd;
	int pid;
	unsigned int groups;
	int MAX_PAYLOAD;
} uu_user_netlink_t;

extern int uuNetlinkOpen (int   *sock_fd, 
				unsigned int  pid, /* If UNDEFINED_INT use getpid(), 
									otherwise use as is */
				unsigned int groups);
extern int uuNetlinkSend (int sock_fd, unsigned int   pid,
				unsigned int groups,
			   /* unsigned int   msgType, unsigned int   seqNo, 
 				* NOT USED; CAN BE USED IN FUTURE
 				*/
				void  *data, long int  dataLen, unsigned int flags);

extern int uuNetlinkRecv (int sock_fd, unsigned int   pid, 
				unsigned int groups,
				/* 	unsigned int msgType, unsigned int  seqNo, 
					NOT USED; CAN BE USED IN FUTURE*/
				void *data, long int *dataLen, unsigned int  flags);

#endif
#endif /* __UU_NETLINK__*/

/* EOF */

