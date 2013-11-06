#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>

#define SWAP(a, b) a ^= b; b ^= a; a ^= b;
#define assert(p) if(!(p)){ fprintf(stderr, "Assertion "#p" line (%d:%s) failed: %s\n", __LINE__, __FILE__, strerror(errno)); return 0; }
size_t RawRead(char*, size_t);
unsigned short checksum(char*, size_t);

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

struct icmp_hdr {
        uint8_t type;
	#define ICMP_ECHOREPLY 0
	#define ICMP_ECHO 8
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
};

int main(void){
	int sock, opt = 1;
	char* buf = malloc(1024);
	struct ip_hdr* A = (struct ip_hdr*)buf;
	struct icmp_hdr* B = (struct icmp_hdr*)(buf + sizeof(struct ip_hdr));
	struct in_addr tmp;
	struct sockaddr_in D;

	assert((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) > 0);
	/* il faut utiliser AF_PACKET pour avoir la couche n°2. */
	assert(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) == 0);

	while(1){
		/* on écoute les messages ICMP. */
		size_t l = read(sock, buf, 1024);
		printf("Lu: %lu octets, type: %d\n", l, A->p & 0xFF);
		if((B->type == ICMP_ECHO)){
			printf("Recu: ICMP ping request\n\tDe: %s\n", inet_ntoa(A->src));
			printf("\tVers: %s\n\tTTL: %d\n", inet_ntoa(A->dst), A->ttl & 0xFF);
			printf("\tID: %04X - Sequence: %04X\n",
				B->un.echo.id, B->un.echo.sequence);

			printf("On répond...\n");
			/* on réutilise le même buffer. */
			/* echange de A->dst et A->src */
			tmp = A->dst;
			A->dst = A->src;
			A->src = tmp;
			/* re-calcul du checksum IP */
			A->checksum = 0;
			A->checksum = checksum((char*)A, sizeof(*A));
			/* on renvoit une réponse... */
			D.sin_addr = A->dst;
			D.sin_port = 0;
			D.sin_family = AF_INET;
			B->type = ICMP_ECHOREPLY;
			B->checksum = 0;
			B->checksum = checksum((char*)B, sizeof(*B));
			assert(sendto(sock, buf, l, 0, (struct sockaddr*)&D, sizeof(D)) >= (signed)l);
		}
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

/* TODO: improve this. You can probably use 64bits or 32bits integers to speed up this. */
unsigned short checksum(char* p, size_t l){
        int sum = 0;
        unsigned short *buf = (unsigned short*)p;
        for(; l > 1; l -= 2, buf++) sum += *buf;
        if(l) sum += *buf << 16;
        sum = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);
        return (unsigned short)~sum;
}
