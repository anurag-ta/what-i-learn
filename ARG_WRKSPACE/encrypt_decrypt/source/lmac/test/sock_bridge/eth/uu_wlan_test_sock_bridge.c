/*
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#if 0
#include <math.h> /* For TPC simulation */
#include<assert.h>
#endif
#include "uu_netlink.h"
#include "uu_phy_stub_nl.h"

#if 0
/** For TPC Simulation */
#define UU_PHY_DISTANCE_TPC_CLIENT      30
static long long int power_consumed = 0;
static long long int num_pkts = 0;

float tpc_mW[180] =
{
    1.047,  1.096,  1.148,  1.202,  1.259,  1.318,  1.38,   1.445,  1.513,  1.584,
    1.66,   1.737,  1.82,   1.90,   1.995,  2.1,    2.187,  2.29,   2.4,    2.51,
    2.63,   2.754,  0,      3.01,   3.162,  3.31,   3.467,  3.63,   3.8,    3.9,
    4.16,   4.365,  4.57,   4.786,  5.0,    5.25,   5.5,    5.75,   6.025,  6.31,
    6.6,    6.91,   7.24,   7.85,   7.94,   8.317,  8.71,   9.12,   9.55,   10,
    10.47,  10.96,  11.48,  12.02,  12.59,  13.18,  13.8,   14.45,  15.14,  15.85,
    16.6,   17.38,  18.2,   19.05,  19.95,  20.89,  21.88,  22.9,   24.0,   25.11,
    26.3,   27.54,  28.84,  30.2,   31.6,   33.11,  34.67,  36.3,   38.0,   40,
    41.7,   43.65,  45.7,   47.87,  50.12,  0,      0,      0,      60.26,  0,
    0,      69.18,  0,      0,      0,      0,      87.1,   0,      0,      0,
    104.71, 0,      0,      0,      0,      0,      138.04, 0,      0,      0,
    0,      0,      0,      190.54, 200,    209,    0,      0,      0,      0,
    263,    0,      0,      0,      0,      0,      347,    0,      0,      0,
    0,      437,    0,      0,      0,      525,    0,      0,      0,      631,
    0,      0,      0,      0,      800,    0,      0,      0,      0,      0,
    1047,   0,      0,      0,      0,      0,      1380,   0,      0,      1585,
    0,      0,      0,      0,      2000,   0,      0,      0,      0,      2511,
    0,      0,      0,      0,      3162,   0,      0,      0,      0,      4000
};
/** -- End of TPC Simulation */
#endif


static int uu_wlan_phy_stub_send_frame(unsigned char *data, int len);
static int send_to_nl(unsigned char *data, int len);
static int recv_from_nl ();

uu_user_netlink_t u_nl_g;
struct sockaddr_in localAddr, remoteAddr;
int sd;

static int send_to_nl(unsigned char *data, int len)
{
    if (u_nl_g.sock_fd == 0)
    {
        printf("NL Socket not open. So droping msg\n");
        return -1;
    }

    printf("Posting msg to NL Socket. msg len %d and data is %x %x %x\n", len,data[0],data[1],data[2]);
    uuNetlinkSend (u_nl_g.sock_fd, u_nl_g.pid, 0, data, len, 0);
    return 0;
}


static int recv_from_nl ()
{
    int ret, i;
    unsigned char data[UU_MAX_PAYLOAD];
    long int datalen;

    datalen = u_nl_g.MAX_PAYLOAD;
    ret = uuNetlinkRecv (u_nl_g.sock_fd, u_nl_g.pid,
        u_nl_g.groups, data, &datalen, 0);
    if (ret == 0)
    {
        printf("NL Thread:  recv fram len: %ld : BYTE:%x %x %x %x\n", datalen, data[0], data[1], data[2], data[3]);
        for(i=0;i<datalen;i++)
        {
            printf("%x ", data[i]);
        }

#if 0
//#ifdef UU_WLAN_TPC

        printf("NITESH SOCK RX PACKET\n");
        printf ("Tx_Pwr = %x\n", data[9]);
        for (i = 0; i < datalen; i++)
            printf("%x ", data[i]);
        printf("\n");
        float rx_power;
        /* PrxdBm = PtxdBm - (10n + C + 10n log (base10) {d/d0])
         * where, d0 is assumed to be 10 metres.
         * C is system loss = 7 dB
         * n is path loss exponent = 2
         *
         * rcvd_pkt[3] = ((rcvd_pkt[3])/5 - (27 + 20 * log10(UU_PHY_DISTANCE_TPC_CLIENT / 10))); (in dBm)
         * Since, the rssi level varies as 0 dBm as 255, and -128 as 0. So, we multiply the rssi (n dBm) with 2
         * and then add 255 to get the rssi level
         */
#if 0
        if (data[12] != 0x80)
        {
            FILE *fp;
            fp = fopen("/home/nitesh/Desktop/tpc_output.txt","a");
            assert(fp);
            for(i = 0; i < datalen; i++)
            fprintf(fp, "%x ", data[i]);
            fprintf(fp, "\n");
            power_consumed += ((datalen - 11) * tpc_mW[data[9] - 1]);
            printf("Power_Consumed in mW is: %lld\n", power_consumed);

            num_pkts ++;
            printf("# of packets: %lld\n", num_pkts);
            if (data[12] == 0x88)
            {
                //if ((data[12] & 0x0c) == 0x08)
                fprintf(fp,"DATA\n");
            }
            fprintf(fp,"num_pkts: %lld\tpower_consumed: %lld\tdata_len: %ld\ttx_pwr: %d\n",num_pkts, power_consumed, datalen, data[9]);
            fclose(fp);
        }
#endif /* #if 0 */
        rx_power = (((2 * (float)data[9])/5) - (2 * (27 + 20 * log10(UU_PHY_DISTANCE_TPC_CLIENT / 10)))) + 255;

        data[9] = (unsigned char) rx_power;

        printf("Rx_Pwr = %x\n", data[9]);
        for(i=0;i<datalen;i++)
            printf("%x ", data[i]);
        printf("\n");

//#endif /* UU_WLAN_TPC */
#endif /* #if 0 */

        printf("\n");
        uu_wlan_phy_stub_send_frame(data, datalen);
    }
    return ret;
} /* recv_from_nl */


static int uu_wlan_phy_stub_send_frame(unsigned char *data, int len)
{

    int ret,n;
    int i;

    for(i=0;i<len;i++)
        printf("%x ", data[i]);
    printf("\n");
    n = sendto(sd,data,len,0,(struct sockaddr *) &remoteAddr, sizeof(struct sockaddr_in));
    if (n < 0)
    {
        printf("sendto failed");
        return 0;
    }
}


int nl_thread_stop_g =0;
void * nl_thread (void *arg)
{
    int ret;

    u_nl_g.groups = UU_NL_PROTO_PHY_STUB;
    u_nl_g.MAX_PAYLOAD = UU_MAX_PAYLOAD;
    u_nl_g.pid = getpid ();

    if (arg != NULL)
        printf("Arg is rcvd");

    if ((ret = uuNetlinkOpen (&u_nl_g.sock_fd, u_nl_g.pid, u_nl_g.groups)) != 0)
    {
        printf("NL socket open failed \n");
        close (u_nl_g.sock_fd);
        return NULL;
    }
    printf("NL Thread Waiting to recv \n");
    while(nl_thread_stop_g == 0)
    {
        recv_from_nl();
    }
    close (u_nl_g.sock_fd);
    u_nl_g.sock_fd = 0;
    return NULL;
} /* nl_thread */


static int uu_wlan_process_rx_pkt(unsigned char *rcvd_pkt, int pktlen)
{
    send_to_nl(rcvd_pkt, pktlen);
    return 0;
}


#define PORT 1500
#define MAX_MSG_SIZE 2000

int main(int argc, char *argv[])
{
    int rc, n, remoteAddrlen, ret;
    //struct sockaddr_in cliAddr, servAddr;
    char msg[MAX_MSG_SIZE];
    pthread_t thread1;
    struct hostent *h;

    /* check command line args */
    if(argc<2)
    {
        printf("usage : %s <server-IP> \n", argv[0]);
        exit(1);
    }

    h = gethostbyname(argv[1]);
    if(h==NULL)
    {
        printf("%s: unknown host '%s' \n", argv[0], argv[1]);
        exit(1);
    }

    printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name, inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

    remoteAddr.sin_family = h->h_addrtype;
    memcpy((char *) &remoteAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    remoteAddr.sin_port = htons(PORT);
    /* socket creation */
    sd = socket(AF_INET,SOCK_DGRAM,0);
    if(sd<0)
    {
        printf("%s: cannot open socket \n",argv[0]);
        exit(1);
    }

    /* bind any port */
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(PORT);

    rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
    if(rc<0)
    {
        printf("cannot bind port number %d \n", PORT);
        exit(1);
    }

    ret = pthread_create( &thread1, NULL, nl_thread, NULL);
    if(ret<0)
    {
        printf(" Thread creation failed \n");
        return ret;
    }


    while(1)
    {
        /* init buffer */
        memset(msg,0x0,MAX_MSG_SIZE);

        /* receive message */
        remoteAddrlen = sizeof(remoteAddr);
        n = recvfrom(sd, msg, MAX_MSG_SIZE, 0, (struct sockaddr *) &remoteAddr, &remoteAddrlen);

        if(n > 0)
        {
            uu_wlan_process_rx_pkt(msg, n);
        }

        /* print received message */
        printf("from %s:UDP%u : %s \n",inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port),msg);

    }/* end of server infinite loop */

    nl_thread_stop_g = 1;
    pthread_join( thread1, NULL);
} /* main */

