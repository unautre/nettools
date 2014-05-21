#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/icmp.h>
#include <netdb.h>
#include <errno.h>

#define assert(p) if(!(p)){ fprintf(stderr, "Assertion "#p" line (%d:%s) failed: %s\n", __LINE__, __FILE__, strerror(errno)); return 0; }

size_t EthRead(char*, size_t);
size_t IpRead(char*, size_t);
size_t TcpRead(char*, size_t);
size_t IcmpRead(char*, size_t);
size_t RawRead(char*, size_t);

typedef uint8_t byte;

struct eth_hdr {
	byte mac_src[6];
	byte mac_dest[6];
	uint16_t type;
};

struct ip_hdr {
	uint8_t vhl;
	uint8_t tos;
	uint16_t len;
	uint16_t id;
	uint16_t flags;
	#define IP_RF 0x8000 /* reserved fragment */
	#define IP_DF 0x4000 /* don't fragment */
	#define IP_MF 0x200 /* more fragments */
	#define IP_OFFMASK 0x1FFF
	uint8_t ttl;
	uint8_t p;
	uint16_t checksum;
	struct in_addr src, dst;
};

struct tcp_hdr {
	uint16_t src_port, dst_port;
	uint32_t seq, ack;
	uint8_t off;
	uint8_t flags;
	#define TCP_FIN 0x01
	#define TCP_SYN 0x02
	#define TCP_RST 0x04
	#define TCP_PUSH 0x08
	#define TCP_ACK 0x10
	#define TCP_URG 0x20
	#define TCP_ECE 0x40
	#define TCP_CWR 0x80
	#define TCP_FLAGS (TCP_FIN|TCP_SYN|TCP_RST|TCP_ACK|TCP_URG|TCP_ECE|TCP_CWR)
	uint16_t winsize;
	uint16_t checksum;
	uint16_t urp;
};

/* struct icmp_hdr {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	union {
		struct {
			uint16_t id;
			uint16_t sequence;
		} echo;
		uint32_t gateway;
		struct {
			uint16_t __pad;
			uint16_t mtu;
		} frag;
	} un;
}; */

int main(int argc, char **argv){
	int sock, j;
	unsigned long i;
	struct sockaddr_in src = {
		.sin_port = 0, .sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_ANY) };
	char *buf = malloc(sizeof(char)*4096); /* where the packets will be stored. */
	char *data = buf;
	struct sockaddr tmp;
	socklen_t tmplen;

	if(getuid() != 0)
		printf("Warning: we're not root.\n");

	assert((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) > 0);
	while(1){
		size_t l = recvfrom(sock, buf, 4096, 0, &tmp, &tmplen);
		if((signed)l <= 0){
			perror("read"); exit(-1); }

		printf("\n\n#### PACKET: %lu bytes.\n", l);
		size_t k = EthRead(buf, l);
		if(k < l) RawRead(buf+k, l - k);
	}

	close(sock);
	return 0;
}

#define PAS 8
size_t RawRead(char* buf, size_t l){
	char* data = buf;
	unsigned i, j;
	size_t old_l = l;

	/* on affiche toutes les données brutes. */
	putchar('\n');
	for(; l > 0; l -= PAS, data += PAS){
		printf("%04X\t", (unsigned int)(data - buf));
		for(i = 0;i<PAS && i < l;i++)
			printf("%02X ", data[i] & 0xFF);
		for(i = 0;i<PAS && i < l;i++)
			printf(isalnum(data[i]) ? "%c" : ".", data[i] & 0xFF);
		putchar('\n');
		if(l <= PAS) break;
	}

	return old_l;
}

size_t EthRead(char* buf, size_t l){
	int i;
	char* data = buf;
	struct eth_hdr* Eth = (struct eth_hdr*)data;

	assert(l > 14);
	/* Affichage du header Ethernet */
	printf("PACKET FROM: %02X", buf[0] & 0xFF);
	for(i = 1;i<6;i++) printf(":%02X", buf[i] & 0xFF);
	printf(" TO %02X", buf[6] & 0xFF);
	for(i = 7;i<12;i++) printf(":%02X", buf[i] & 0xFF);
	printf(" type: %04X %s\n", Eth->type, (Eth->type == 8) ? "(IP)" : "");
	data += sizeof(struct eth_hdr);

	if(Eth->type == 8)
		data += IpRead(data, l - sizeof(struct eth_hdr));

	return (size_t)(data - buf);
}

size_t IpRead(char* buf, size_t l){
	int i;
	char* data = buf;
	struct ip_hdr* Ip = (struct ip_hdr*)data;

	assert(l > sizeof(struct ip_hdr));
	/* Affichage du header IP */
	printf("IPv%d - %d bytes total (%d for header) - TTL: %d - protocol: %d\n",
		(Ip->vhl >> 4), ntohs(Ip->len), (Ip->vhl & 0x0F)*4, Ip->ttl, Ip->p);
	printf("Flags: ");
	if(Ip->flags & IP_RF) printf("RF ");
	if(Ip->flags & IP_DF) printf("DF ");
	if(Ip->flags & IP_MF) printf("MF ");

	printf("- From %s", inet_ntoa(Ip->src));
	printf(" to %s\n", inet_ntoa(Ip->dst));
	data += sizeof(struct ip_hdr);

	switch(Ip->p){
		case 6:
			data += TcpRead(data, l - sizeof(struct ip_hdr)); break;
		case 1:
			data += IcmpRead(data, l - sizeof(struct ip_hdr)); break;
		default:
			break;
	}

	return (size_t)(data - buf);
}

size_t TcpRead(char* buf, size_t l){
	int i;
	char* data = buf;
	struct tcp_hdr* Tcp = (struct tcp_hdr*)data;

	assert(l > sizeof(struct tcp_hdr));
	/* Affichage du header tcp. */
	printf("Port %d to %d - flags:", ntohs(Tcp->src_port), ntohs(Tcp->dst_port));
	if(Tcp->flags & TCP_FIN) printf("FIN");
	if(Tcp->flags & TCP_SYN) printf("SYN");
	if(Tcp->flags & TCP_RST) printf("RST");
	if(Tcp->flags & TCP_PUSH) printf("PUSH");
	if(Tcp->flags & TCP_ACK) printf("ACK");
	if(Tcp->flags & TCP_URG) printf("URG");
	if(Tcp->flags & TCP_ECE) printf("ECE");
	if(Tcp->flags & TCP_CWR) printf("CWR");
	data += sizeof(struct tcp_hdr);

	return (size_t)(data - buf);
}

size_t IcmpRead(char* buf, size_t l){
	int i;
	char* data = buf;
	struct icmphdr* Icmp = (struct icmphdr*)data;

	assert(l > sizeof(struct icmphdr));
	/* Affichage du header ICMP */
	printf("ICMP PACKET: type(%d) code(%d)\n", Icmp->type, Icmp->code);
	switch(Icmp->type){
		case ICMP_ECHOREPLY:
			printf("PING REPLY: %d/%d\n", Icmp->un.echo.id,
				Icmp->un.echo.sequence); break;
		case ICMP_ECHO:
			printf("PING REQUEST: %d/%d\n", Icmp->un.echo.id,
				Icmp->un.echo.sequence); break;
		default:
			break;
	}

	data += sizeof(struct icmphdr);
	return (size_t)(data - buf);
}

/* TODO: improve this. You can probably use 64bits or 32bits integers to speed up this. */
unsigned short checksum(char* p, size_t l){
	int sum = 0;
	unsigned short *buf = (unsigned short*)p;
	for(; l > 1; l -= 2, buf++) sum += *buf;
	if(l) sum += *buf << 16;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (unsigned short)~sum; }
