#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>

#define assert(p) if(!(p)){ perror("Erreur à l\'assertion "#p"--->"); exit(5); }

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
void sendping(int sock, uint16_t id, uint16_t sequence, char* message, size_t msglen, struct sockaddr_in* dest);

int main(int argc, char** argv){
	int sock;
	struct hostent *hname;
	struct sockaddr_in dest, src;
	socklen_t slen = sizeof(src);
	char buf[1024];
	struct packet *pack;

	if(argc < 2){
		printf("Usage: %s address\n", argv[0]);
		return 0; }

	hname = gethostbyname(argv[1]);
	dest.sin_family = AF_INET;
	dest.sin_port = 0; /* ICMP, pas de port. */
	dest.sin_addr = *((struct in_addr*)(hname->h_addr_list[0])); /* should use gethostbyname instead */
	assert(sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP));

	/* on va envoyer un paquet ICMP ping. */
	sendping(sock, (uint16_t)0xD34D, (uint16_t)0xBEEF, "hello world !\n", 14, &dest);

	src.sin_family = AF_INET;
	src.sin_port = 0;
	src.sin_addr.s_addr = htonl(INADDR_ANY);
	assert(bind(sock, (struct sockaddr*)&src, sizeof(src)) == 0);
	/* on va écouter les paquets recus et vérifier si l'ID et la séquence correspondent au notre. */
	while(1){
		size_t l = recvfrom(sock, buf, sizeof(buf), MSG_WAITALL, (struct sockaddr*)&src, &slen);
		pack = (struct packet*)(buf+(5*4)); /* we skip the IP header */
		printf("PACKET! type:%d code:%d size:%lu\n\tid:%04X seq:%04X\n", pack->hdr.type, pack->hdr.code,
			l, pack->hdr.echo.id, pack->hdr.echo.sequence);
	}

	close(sock);
	return 0;
}

void sendping(int sock, uint16_t id, uint16_t sequence, char* message, size_t msglen, struct sockaddr_in* dest){
	size_t len;
	struct packet *pack = calloc(len = sizeof(pack->hdr) + msglen, sizeof(char));
	pack->hdr.type = 8;
	pack->hdr.code = 0;
	pack->hdr.checksum = 0;
	pack->hdr.echo.id = id;
	pack->hdr.echo.sequence = sequence;
	memcpy(pack->msg, message, msglen);
	pack->hdr.checksum = checksum(pack, len);
	assert(sendto(sock, pack, len, 0, (struct sockaddr*)dest, sizeof(*dest)) > 0);
	free(pack);
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
