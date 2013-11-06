#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

// http://www.univ-tln.fr/backend-breves.php3
void automate(FILE* f);

int main(int argc, char **argv){
	FILE *f;
	char *host = NULL, *uri = NULL;
	int sock = -1, etat = 0, c;
	struct addrinfo *res, *it;

	if(argc <= 1)
		f = stdin;
	else{
		host = strtok_r(argv[1], "/", &uri);
		if(strcmp(host, "http:") == 0 || strcmp(host, "https:") == 0)
			host = strtok_r(NULL, "/", &uri);
		if((c = getaddrinfo(host, "80", NULL, &res))){
			fprintf(stderr, "Erreur: %s\n", gai_strerror(c));
			exit(-1); }
		for(it = res; it != NULL; it = it->ai_next){
			sock = socket(it->ai_family, it->ai_socktype,
				it->ai_protocol);
			if(connect(sock, it->ai_addr, it->ai_addrlen) == 0)
				break;
			close(sock);
		}
		if(it == NULL){
			fprintf(stderr, "Erreur: connexion impossible.\n");
			exit(-1); }
		freeaddrinfo(res);

		f = fdopen(sock, "r+");
		fprintf(f, "GET /%s\n\n", uri);
	}

	while((c = getc(f)) != EOF){
		switch(etat){
			case 0: etat = (c == '<') ? 1 : 0; break;
			case 1: etat = (c == 't') ? 2 : 0; break;
			case 2: etat = (c == 'i') ? 3 : 0; break;
			case 3: etat = (c == 't') ? 4 : 0; break;
			case 4: etat = (c == 'l') ? 5 : 0; break;
			case 5: etat = (c == 'e') ? 6 : 0; break;
			case 6: etat = (c == '>') ? 7 : 0; break;
			case 7: if(c == '<'){ etat = 0; putchar('\n'); }
				else putchar(c); break;
			default: break;
		}
	}
	return 0;
}
