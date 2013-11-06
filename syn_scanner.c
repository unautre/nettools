#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <signal.h>

#define assert(p) if(!(p)){ perror("Assertion "#p" failed"); exit(-1); }

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

struct pseudo_hdr {
	uint32_t src_addr, dst_addr;
	uint8_t mbz, proto;
	uint16_t len;
};

struct tcp_hdr {
	uint16_t source_port;
	uint16_t dest_port
	uint32_t seq;
	uint32_t ack;
	#define TCPOFFSET(x) ((x & 0xf0) >> 4)
	uint8_t off;
	uint8_t flags;
	#define TH_FIN	0x01
	#define TH_SYN	0x02
	#define TH_RST	0x04
	#define TH_PUSH	0x08
	#define TH_ACK	0x10
	#define TH_URG	0x20
	#define TH_ECE	0x40
	#define TH_CWR	0x80
	#define TH_FLAGS	(TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	uint16_t win_size;
	uint16_t checksum;
	uint16_t urgent_pointer;
};

struct sockaddr getaddr(char*);
struct sockaddr get
void handler(int);
unsigned int checksum(char*, size_t);

int main(int argc, char **argv){
	struct addrinfo *res, *it, hints = {
		.ai_family = AF_INET, .ai_socktype = 0,
		.ai_protocol = 0, .ai_flags = 0 };
	struct sockaddr_in addr;
	socklen_t slen;
	char buf[1024];
	int i, sock;

	if(argc < 2){
		printf("Usage: %s server [port|port range]\n", argv[0]); return 0; }

	if((i = getaddrinfo(argv[1], NULL, &hints, &res))){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i)); return 1; }

	assert((sock = socket(AF_INET, SOCK_RAW, 0)) <= 0);

	if(fork() == 0){
		/* fils, on envoit les requetes. */
		int a, b;
		struct pseudo_hdr *pP = (struct pseudo_hdr*)buf;
		struct tcp_hdr* P = (struct tcp_hdr*)buf + sizeof(*pP);
			
		if(sscanf(argv[1], "%d-%d", &a, &b) == 1)
			b = a;

		while(a <= b){
			P->source_port = rand() % 0xFFFF;
			P->dest_port = htons(a);
			P->seq = rand() % 0xFFFF;
			P->ack = 0;
			P->off = TCPOFFSET(5);
			P->flags = TH_SYN;
			P->win_size = 65535;
			P->checksum = 0;
			P->urgent_pointer = 0;
			pP->src; /* il faut récuperer notre IP. getaddrinfo ? */
			P->checksum = checksum(buf, sizeof(*P) + sizeof(*pP));
	}else{
		/* pere, on recoit les paquets. */
	}

	return 0;
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
