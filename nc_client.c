#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>

#define assert(p) if(!(p)){ perror("Assertion non vérifiée:\n\t->"#p"\n\tErreur"); exit(__LINE__); }
#define BUFLEN 256

/* TODO: simple telnet proxy. */
/* YET: just a  telnet client */
int main(int argc, char **argv){
	char buf[BUFLEN];
	struct addrinfo *res, *it, hints;
	int sock, i;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_canonname = NULL;

	assert(argc > 2);
	assert((sock = socket(AF_INET, SOCK_STREAM, 0)) != 0);
	assert(getaddrinfo(argv[1], argv[2], &hints, &res) == 0);

	for(it = res; it != NULL; it = it->ai_next)
		if(connect(sock, res->ai_addr, res->ai_addrlen) != 0)
			break;
	if(it == NULL){
		printf("Erreur: impossible de se connecter.\n");
		exit(-1); }

	/* Si on lit sur stdin, on écrit sur sock. */
	/* Si on lit sur sock, on écrit sur stdout. */
	/* donc, on check sock et stdin pour lecture. */
	fd_set readfds, writefds, errorfds;
	while(1){
		FD_ZERO(&readfds);
		FD_ZERO(&errorfds);
		FD_SET(sock, &readfds);
		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(sock, &errorfds);

		/* on lance la primitive select() */
		select(sock+1, &readfds, NULL, &errorfds, NULL);
		if(FD_ISSET(sock, &errorfds))
			perror("socket");
		if(FD_ISSET(sock, &readfds))
			write(STDOUT_FILENO, buf, read(sock, buf, sizeof(buf)));
		if(FD_ISSET(STDIN_FILENO, &readfds))
			write(sock, buf, read(STDIN_FILENO, buf, sizeof(buf)));
	}

	return 0;
}
