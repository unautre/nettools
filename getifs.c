#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

char* addr2str(struct sockaddr* in){
	static char buf[INET6_ADDRSTRLEN];

	if(in == NULL){
		strcpy(buf, "None.");
	}else
	switch(in->sa_family){
		case AF_INET:
			inet_ntop(AF_INET, &(((struct sockaddr_in*)in)->sin_addr),
				buf, sizeof(buf)); break;
		case AF_INET6:
			inet_ntop(AF_INET6, &(((struct sockaddr_in6*)in)->sin6_addr),
				buf, sizeof(buf)); break;
		default:
			strcpy(buf, "Error"); break;
	}
	return buf;
}

int main(void){
	struct ifaddrs *res = NULL, *it;

	if(getifaddrs(&res)){
		perror("getifaddrs"); exit(-1); }

	for(it = res; it != NULL; it = it->ifa_next){
		printf("Interface %s:\n", it->ifa_name);
		printf("\tAddress: %s\n", addr2str(it->ifa_addr));
		printf("\tNetmask: %s\n", addr2str(it->ifa_netmask));

		/* SIOCGIFFLAGS */
		#define _(FLAG) if(it->ifa_flags & FLAG) printf("\t\t" #FLAG "\n");
		_(IFF_UP)
		_(IFF_BROADCAST)
		_(IFF_DEBUG)
		_(IFF_LOOPBACK)
		_(IFF_POINTOPOINT)
		_(IFF_RUNNING)
		_(IFF_NOARP)
		_(IFF_PROMISC)
		_(IFF_NOTRAILERS)
		_(IFF_ALLMULTI)
		_(IFF_MASTER)
		_(IFF_SLAVE)
		_(IFF_MULTICAST)
		_(IFF_PORTSEL)
		_(IFF_AUTOMEDIA)
		_(IFF_DYNAMIC)
		// _(IFF_LOWER_UP)
		// _(IFF_DORMANT)
		// _(IFF_ECHO)
	}

	freeifaddrs(res);

	return 0;
}
