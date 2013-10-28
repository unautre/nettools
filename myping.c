#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>

#define assert(p) if(!(p)){ perror("Erreur à l\'assertion "#p"\n\tMessage"); \
	exit(5); }

struct icmp_packet {
	struct header {
		uint8_t type, code;
		uint16_t checksum;
		struct {
			uint16_t id, sequence;
		} echo;
	} hdr;
	#if __STDC_VERSION__ >= 199901L
	char msg[]; /* requires C99 */
	#else
	char msg[1];
	#endif
};

uint16_t checksum(struct packet*, size_t);
void sendping(int sock, uint16_t id, uint16_t seq, char* msg, size_t msglen,
	 struct sockaddr_in* dest);

int listener;
struct sockaddr_in dest;

int main(int argc, char **argv){
	/* usage de getopt ? */
	/* usage de (p)thread ou fork() ? */
	/* fonction pingpong, qui enverra le message et recevra la réponse ? */
	/* on 
