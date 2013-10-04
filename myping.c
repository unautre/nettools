#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>

#define assert(p) if(!(p)){ perror("Erreur à l\'assertion "#p"\n\tMessage"); exit(5); }

struct packet {
	struct header {
		uint8_t type;
		uint8_t code;
		uint16_t checksum;
		struct {
			uint16_t id;
			uint16_t sequence;
		} echo;		
	} hdr;
	#if __STDC_VERSION__ >= 199901L
	/* requires C99 */
	char msg[];
	#endif
};

unsigned short checksum(struct packet*, size_t);

int main(int argc, char** argv){
	int sock;
	struct hostent *hname;
	struct protoent *proto;
	struct sockaddr_in dest;
	struct packet *pack;
	size_t packlen;

	if(argc < 2){
		printf("Usage: %s address\nRight now, it only sends PING packet, doesn't receive them.\n", argv[0]);
		return 0; }

	proto = getprotobyname("ICMP");
	hname = gethostbyname(argv[1]);
	dest.sin_family = AF_INET;
	dest.sin_port = 0; /* ICMP, pas de port. */
	dest.sin_addr.s_addr = inet_addr(argv[1]); /* should use gethostbyname instead */
	assert(sock = socket(AF_INET, SOCK_RAW, proto->p_proto)); /* probably IPPROTO_ICMP */

	/* on va envoyer un paquet ICMP ping. */
	pack = calloc(packlen = 64, sizeof(char));
	pack->hdr.type = 8; /* ECHO message */
	pack->hdr.code = 0;
	pack->hdr.checksum = 0;
	pack->hdr.echo.id = 0xD34D;
	pack->hdr.echo.sequence = 0xBEEF;
	sprintf(pack->msg, "abcdefABCDEF");
	pack->hdr.checksum = checksum(pack, packlen);
	assert(sendto(sock, pack, packlen, 0, (struct sockaddr*)&dest, sizeof(dest)) > 0);

	close(sock);
	return 0;
}

/* TODO: improve this. You can probably use 64bits or 32bits integers to speed up this. */
unsigned short checksum(struct packet* p, size_t l){
	int sum = 0;
	unsigned short *buf = (unsigned short*)p;
	for(; l > 1; l -= 2, buf++) sum += *buf;
	if(l) sum += *buf << 16;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (unsigned short)~sum;
}