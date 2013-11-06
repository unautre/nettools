#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#if INET_ADDRSTRLEN > INET6_ADDRSTRLEN
#define BUFLEN INET_ADDRSTRLEN
#else
#define BUFLEN INET6_ADDRSTRLEN
#endif

int main(int argc, char **argv){
	char buf[BUFLEN+1] = "";
	struct addrinfo* res = NULL;
	struct addrinfo hints;
	int status, i;

	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	status = getaddrinfo(argv[1], NULL, &hints, &res);
	if(status) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));

	for(;res != NULL; res = res->ai_next){
		char* inp_addr = (char*)res->ai_addr + 4; /* sale ! */
		if(inet_ntop(res->ai_family, inp_addr, buf, sizeof(buf)) == NULL)
			perror("inet_ntop");
		else
			printf("---> Adresse: %s\t%s\n", buf, res->ai_canonname);
	}

	return 0;
}
