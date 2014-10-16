#include<stdio.h>
#include<stdlib.h>
//#include<memory.h>
//#include<errno.h>
#include<string.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <pthread.h>
#include <unistd.h>

#include "uu_return_types.h"
#include "uu_netlink.h"

#define UU_UNDEFINED_INT 0XFFFF

int 
uuNetlinkOpen (int   *sock_fd, 
	unsigned int  pid, /* If UNDEFINED_INT use getpid(), otherwise use as is */
	unsigned int groups)
{
	struct sockaddr_nl  src_addr;

	if (sock_fd == NULL)
	 {
		return UU_FAILURE;
	 }

	/* TODO: Why SOCK_RAW ?? */
	*sock_fd = socket (PF_NETLINK, SOCK_RAW, UU_NL_PROTO_PHY_STUB);
	if (*sock_fd < 0)
	 {
		/* Socket create failed */
		printf("NETLINK: socket create failed\n");
		return UU_FAILURE;
	 }
	//UU_AFDX_LOG_DEBUG(("NETLINK: socket %d open success\n",  *sock_fd));
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	if (pid == UU_UNDEFINED_INT)
	 {
		//src_addr.nl_pid = getpid();  /* self pid */
		src_addr.nl_pid = pthread_self () << 16 | getpid ();  /* self threadId + pid */
	 }
	else
	 {
		src_addr.nl_pid = pid;
	 }
	src_addr.nl_groups = groups;  /* not in mcast groups */

	if (bind (*sock_fd, (struct sockaddr*)&src_addr, 
			(socklen_t)sizeof (src_addr)) < 0)
	 {
		/* Bind failed */
  	 	printf("NETLINK: bind failed\n");
		close (*sock_fd);
//		*sock_fd = UU_INVALID_SOCKET;
		return UU_FAILURE;
	 }
	printf("NETLINK: socket %d bind success\n",  *sock_fd);

	return UU_SUCCESS;
} /* uuNetlinkOpen */


int 
uuNetlinkSend (int sock_fd, unsigned int pid, unsigned int groups,
	/*	unsigned int	msgType, NOT USED; CAN BE USED IN FUTURE*/
	/*	unsigned int	seqNo, NOT USED; CAN BE USED IN FUTURE*/
		void  *data, long int  dataLen,	
		unsigned int flags)
{
	struct sockaddr_nl dest_addr;
	struct msghdr msg;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	void *tmp = NULL;

	if ((data == NULL) || (dataLen <= 0))
	 {
		/* Inavlid Inputs */
//		UU_AFDX_LOG_DEBUG(("NETLINK: Invalid input in netlinkSend\n"));
		return UU_INVALVAL;
	 }

	if (dataLen > UU_MAX_PAYLOAD)
	 {
		/* Inavlid Inputs */
//		UU_AFDX_LOG_DEBUG(("NETLINK: Msg is too big to send. max allowed: %d reqested:%ld\n", UU_AFDX_MAX_PAYLOAD, dataLen));
		return UU_FAILURE;
	 }

	memset (&dest_addr, 0, sizeof (dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;	/* For Linux Kernel */
	dest_addr.nl_groups = groups; /* unicast */

	/* TODO: Laxman */
#if 0
	nlh=(struct nlmsghdr *)calloc(1, NLMSG_SPACE(UU_AFDX_MAX_PAYLOAD));
	if (nlh == NULL)
	{
		UU_AFDX_LOG_DEBUG(("NETLINK: Malloc failed\n"));
		return UU_FAILURE;
	}

	/* Filling the netlink message header */
	nlh->nlmsg_len = NLMSG_SPACE(UU_AFDX_MAX_PAYLOAD);
#endif
	nlh = (struct nlmsghdr *)calloc (1, NLMSG_SPACE (dataLen));
	if (nlh == NULL)
	 {
//		UU_AFDX_LOG_DEBUG(("NETLINK: Malloc failed\n"));
		return UU_FAILURE;
	 }

	/* Filling the netlink message header */
	//nlh->nlmsg_len = NLMSG_SPACE (dataLen);
	nlh->nlmsg_len = NLMSG_LENGTH(dataLen);
	if (pid == UU_UNDEFINED_INT)
	 {
		nlh->nlmsg_pid = pthread_self () << 16 | getpid();//sock_fd.getpid();
	 }
	else 
	 {
		nlh->nlmsg_pid = pid;
	 }
	nlh->nlmsg_flags = flags; /* Relook the flags */
//	nlh->nlmsg_type =  msgType;
//	nlh->nlmsg_seq =	seqNo;
	/* Filling the netlink message payload */
	tmp = NLMSG_DATA (nlh);
	memcpy (tmp, data, dataLen);

	memset(&iov, 0, sizeof (struct iovec));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset (&msg, 0, sizeof (struct msghdr));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof (dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

#if 0
	{
		uuAfdxNetLinkMsg_t *debug = tmp;
		UU_AFDX_LOG_DEBUG(("DEBUG: cmd:%d sock:%d \n", debug->cmd, debug->sockFd));
		if (debug->cmd == 2)
			UU_AFDX_LOG_DEBUG(("DEBUG: Msg:%s	Len:%d \n", debug->u.SampWrite.msg, debug->u.SampWrite.msgLen));
		  
		UU_AFDX_LOG_DEBUG(("DEBUG: sending msg Len:%d nlh Msg Len:%d iov len:%d \n", dataLen, nlh->nlmsg_len, iov.iov_len));
	}
#endif

	/* Send Message to Kernel */
	if (sendmsg (sock_fd, &msg, 0) < 0)
	{
		/* Send Failed */
//		UU_AFDX_LOG_ERROR(("NLNK: Send error. Error(%d) pid(%d) MsgSeq(%d) MsgType(%d)", 
//				errno, nlh->nlmsg_pid, nlh->nlmsg_seq, nlh->nlmsg_type));
		free (nlh);
		return UU_FAILURE;
	}

#if 0
	/* Read reply message from kernel */
	memset(nlh, 0, NLMSG_SPACE(dataLen)); /* TODO: use seperate nlh instead of sending nlh */
	recvmsg(sock_fd, &msg, 0);

	tmp = NLMSG_DATA(nlh);
	data_len = nlh->nlmsg_len - NLMSG_LENGTH(0);
	memcpy(data, tmp, *dataLen);
	if (data_len <= *dataLen)
	{
		memcpy(data, tmp, data_len);
		*dataLen = data_len;
	}
	else
	{
		/* Not enough memroy to copy msg */
		return UU_FAILURE;
	}
#endif
	free (nlh);
	return UU_SUCCESS;
} /* uuNetlinkSend */


int 
uuNetlinkRecv ( int sock_fd, unsigned int pid, unsigned int groups,
  /* unsigned int	msgType, NOT USED; CAN BE USED IN FUTURE*/
  /* unsigned int	seqNo, NOT USED; CAN BE USED IN FUTURE*/
	void *data, long int *dataLen, 
	unsigned int flags)
{
	struct sockaddr_nl dest_addr;
	struct msghdr msg;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	void *tmp = NULL;
	int data_len;

	if ((dataLen == NULL) || (*dataLen == 0) ||
		 (data == NULL))
	 {
		/* Inavlid Inputs */
		//UU_AFDX_LOG_DEBUG(("NETLINK: Invalid input in netlinkRecv\n"));
		return UU_INVALVAL;
	 }

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;	/* For Linux Kernel */
	dest_addr.nl_groups = groups; /* unicast */

	/* TODO: Laxman */
#if 0
	nlh=(struct nlmsghdr *)calloc(1, NLMSG_SPACE(UU_AFDX_MAX_PAYLOAD));
	if (nlh == NULL)
	{
		UU_AFDX_LOG_DEBUG(("NETLINK: Malloc failed\n"));
		return UU_FAILURE;
	}

	/* Filling the netlink message header */
	nlh->nlmsg_len = NLMSG_SPACE(UU_AFDX_MAX_PAYLOAD);
#endif
	nlh = (struct nlmsghdr *)calloc (1, NLMSG_SPACE (*dataLen));
	if (nlh == NULL)
	 {
//		UU_AFDX_LOG_DEBUG(("NETLINK: Malloc failed\n"));
		return UU_FAILURE;
	 }

	/* Filling the netlink message header */
	nlh->nlmsg_len = NLMSG_SPACE(*dataLen);
	if (pid == UU_UNDEFINED_INT)
	 {
		nlh->nlmsg_pid = pthread_self () << 16 | getpid ();//sock_fd.getpid();
	 }
	else 
	 {
		nlh->nlmsg_pid = pid;
	 }
	nlh->nlmsg_flags = flags; /* Relook the flags */
//	nlh->nlmsg_type =  msgType;
//	nlh->nlmsg_seq =	seqNo;

	memset(&iov, 0, sizeof(struct iovec));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

#if 0
	{
		uuAfdxNetLinkMsg_t *debug = tmp;
		UU_AFDX_LOG_DEBUG(("DEBUG: cmd:%d sock:%d \n", debug->cmd, debug->sockFd));
		if (debug->cmd == 2)
			UU_AFDX_LOG_DEBUG(("DEBUG: Msg:%s	Len:%d \n", debug->u.SampWrite.msg, debug->u.SampWrite.msgLen));
		  
		UU_AFDX_LOG_DEBUG(("DEBUG: sending msg Len:%d nlh Msg Len:%d iov len:%d \n", dataLen, nlh->nlmsg_len, iov.iov_len));
	}
#endif

	/* Read reply message from kernel */
	if (recvmsg (sock_fd, &msg, 0) < 0)
	 {
		/* Recv Failed */
	  // UU_AFDX_LOG_ERROR(("NLNK: Recv error. Error(%d) pid(%d) ", 
	//			errno, nlh->nlmsg_pid));
		/* TODO: Free the allocated memory */
		free (nlh);
		return UU_FAILURE;
	 }
	tmp = NLMSG_DATA (nlh);
	data_len = nlh->nlmsg_len - NLMSG_LENGTH(0);
//printf("\nLAXMAN after rcv datalen:%d nlmsg_len:%d \n", data_len, nlh->nlmsg_len);
	//memcpy (data, tmp, *dataLen);
	if (data_len <= *dataLen)
	{
		memcpy(data, tmp, data_len);
		*dataLen = data_len;
	}
	else
	{
		/* Not enough memroy to copy msg */
		memcpy(data, tmp, *dataLen);
		*dataLen = data_len;
		return UU_SUCCESS; //TODO:: return data truncated 
	}
	free (nlh);
	return UU_SUCCESS;
} /* uuNetlinkRecv */


/* EOF */

