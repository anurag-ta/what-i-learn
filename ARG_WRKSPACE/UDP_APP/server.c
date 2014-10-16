#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
//#include <delay.h>

#define BUFLEN 1024
#define PORT 9930
#define NUM_PACK 100000
void diep(char *s)
 {
   perror(s);
   exit(1);
 }

int main(void)
 {
   struct sockaddr_in si_me, si_other;
   int s, i, slen=sizeof(si_other);
   unsigned char buf[BUFLEN];
    int iTos = 0x28,ret = 0;

   if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
     diep("socket");

   memset((char *) &si_me, 0, sizeof(si_me));
   si_me.sin_family = AF_INET;
   si_me.sin_port = htons(PORT);
   si_me.sin_addr.s_addr = htonl(INADDR_ANY);
   if (bind(s, &si_me, sizeof(si_me))==-1)
       diep("bind");


   ret = setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&iTos, sizeof(iTos));

   for (i=0; i<NUM_PACK; i++) {
     if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
       diep("recvfrom()");
     printf("Received packet from %s:%d\nData: %s\n\n", 
            inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
}

 close(s);
 return 0;
}
