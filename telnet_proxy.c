#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>

// #define assert(p) if(!(p)){ perror("Assertion non vérifiée:\n\t->"#p"\n\tErreur"); exit(__LINE__); }
#define BUFLEN 512
#define MAXCLIENTS 5

int open_client(char*, char*);
int open_server(char* port, int maxclients);

int main(int argc, char **argv){
	char buf[BUFLEN], buf2[BUFLEN], addrbuf[INET_ADDRSTRLEN+1];
	int sock, i, j, clients[MAXCLIENTS] = {0, 0, 0, 0, 0}, max;
	size_t len; socklen_t x;
	fd_set readfds, errorfds;
	struct sockaddr_in addr[MAXCLIENTS];

	assert(argc > 1);
	sock = max = open_server(argv[1], MAXCLIENTS);
	while(1){
		FD_ZERO(&readfds);
		FD_ZERO(&errorfds);
		for(i = 0;i<MAXCLIENTS;i++)
			if(clients[i]){
				FD_SET(clients[i], &readfds);
				FD_SET(clients[i], &errorfds);
				if(clients[i] > max) max = clients[i]; }
		FD_SET(sock, &readfds);
		FD_SET(sock, &errorfds);
		select(max+1, &readfds, NULL, &errorfds, NULL);
		if(FD_ISSET(sock, &errorfds)){
			perror("socket"); return -1; }
		if(FD_ISSET(sock, &readfds)){
			for(i = 0;i<MAXCLIENTS;i++)
				if(clients[i]){
					clients[i] = accept(sock, (struct sockaddr*)addr+i, &x);
					break; }
			if(i == MAXCLIENTS){
				int s = accept(sock, NULL, NULL);
				write(s, "Trop de connexions.\n", 20);
				close(s); }
		}
		for(i = 0;i<MAXCLIENTS;i++){
			if(FD_ISSET(clients[i], &readfds)){
				len = read(clients[i], buf, BUFLEN);
				inet_ntop(AF_INET, (void*)&addr[i].sin_addr, addrbuf, sizeof(addrbuf));
				len = sprintf(buf2, "<%s> wrote: %*s\n", addrbuf, (int)len, buf);
				for(j = 0;j<MAXCLIENTS;i++){
					if(i != j && clients[i])
						write(clients[i], buf2, len); }
			}
			if(FD_ISSET(clients[i], &errorfds)){
				close(clients[i]);
				clients[i] = 0; }
		}		
	}
	return 0;
}

int open_client(char* host, char* port){
	int sock;
	struct addrinfo *res, *it, hints = { 0, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL};

	assert((sock = socket(AF_INET, SOCK_STREAM, 0)) != 0);
	assert(getaddrinfo(host, port, &hints, &res) == 0);
	for(it = res; it != NULL; it = it->ai_next)
		if(connect(sock, res->ai_addr, res->ai_addrlen) == 0)
			break;

	if(it == NULL){
		printf("Erreur: impossible de se connecter.\n");
		exit(-1);
	}
	return sock; }

int open_server(char* port, int maxclients){
	int sock;
	struct addrinfo *res, *it, hints = { AI_PASSIVE, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL};

	assert((sock = socket(AF_INET, SOCK_STREAM, 0)) != 0);
	assert(getaddrinfo(NULL, port, &hints, &res) == 0);
	for(it = res; it != NULL; it = it->ai_next)
		if(bind(sock, it->ai_addr, res->ai_addrlen) == 0) break;
	if(it == NULL){
		printf("Erreur: impossible de se connecter.\n");
		exit(-1);
	}
	assert(listen(sock, maxclients) == 0);
	return sock; }
