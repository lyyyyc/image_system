#include <stdio.h>
#include<ifaddrs.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main()
{
    struct ifaddrs *addrs;
    getifaddrs(&addrs);
    while(addrs!=NULL)
    {
        struct sockaddr_in *ip=(struct sockaddr_in*)addrs->ifa_addr;
        struct sockaddr_in *mask=(struct sockaddr_in*)addrs->ifa_netmask;
        if(ip->sin_family!=AF_INET)
        {
            addrs=addrs->ifa_next;
            continue;
        }
        if(ip->sin_addr.s_addr==inet_addr("127.0.0.1"))
        {
            addrs=addrs->ifa_next;
            continue;
        }
        printf("name:%s\n",addrs->ifa_name);
        printf("ip:%s\n",inet_ntoa(ip->sin_addr));
        printf("mask:%s\n",inet_ntoa(mask->sin_addr));
        uint32_t net=ntohl(ip->sin_addr.s_addr&mask->sin_addr.s_addr);
        uint32_t host=ntohl(~mask->sin_addr.s_addr);
        int i;
        for(i=1;i<host;++i)
        {
            struct in_addr ip;
            ip.s_addr=htonl(net+i);
            printf("ip:%s\n",inet_ntoa(ip));
        }
        addrs=addrs->ifa_next;
    }
    return 0;
}

