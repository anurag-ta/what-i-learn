 #define SRV_IP "10.0.0.4"

 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <stdio.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <unistd.h>

 #define BUFLEN 1024
 #define PORT 9930
 #define NUM_PACK 100000
 #define DELAY 30

void diep(char *s)
  {
    perror(s);
    exit(1);
  }

 
int main(void)
  {
    struct sockaddr_in si_other;
    int s,np=1,i,j, slen=sizeof(si_other);
    unsigned char buf[BUFLEN];
    int ret, delay, iTos = 64,bk=0,be=64,vi=128,vo=192;
    int so_priority = 128;

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      diep("socket");

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
      fprintf(stderr, "inet_aton() failed\n");
      exit(1);
    }

    //ret = setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&iTos, sizeof(iTos));

    int opt = so_priority << 5;	
 //   setsockopt(mysocks[i], SOL_UDP, IP_TOS, &opt, sizeof(opt));

 //   setsockopt(s, SOL_SOCKET, SO_PRIORITY, &so_priority, sizeof(so_priority));


    for (i=0; i < NUM_PACK; i++) {
      printf("Sending packet %d\n", i);
      sprintf(buf, "This is packet %d \n", i);
      /*for (j= 0;j < BUFLEN; j++)
	{
	printf("%02x ",buf[j]);	
	}*/
      printf("\n\n");
      usleep(DELAY*1000);
		
ret = setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&vi, sizeof(iTos));
      if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
        diep("sendto()");
ret = setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&be, sizeof(iTos));
      if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
        diep("sendto()");

#if 1	
ret = setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&bk, sizeof(iTos));
      if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
        diep("sendto()");

ret = setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&vo, sizeof(iTos));
      if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
        diep("sendto()");
#endif
    }

    close(s);
    return 0;
  }
