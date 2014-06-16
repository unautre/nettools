#include <ext/stdio_filebuf.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

using namespace std;

int main(void){
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	// FILE* f = fdopen(sock, "r+");
	__gnu_cxx::stdio_filebuf<char> filebuf(sock, std::ios::out);
	ostream flux(&filebuf);

	struct addrinfo *res, *it, hints = {
		0,		// ai_flags
		AF_INET,	// ai_family
		SOCK_STREAM,	// ai_socktype
		0,		// ai_protocol
		0,		// ai_addrlen
		NULL,		// ai_addr
		NULL,		// ai_canonname
		NULL,		// ai_next
	};
		
	getaddrinfo("localhost", "1337", &hints, &res);
	for(it = res; it != NULL; it = it->ai_next)
		if(connect(sock, res->ai_addr, res->ai_addrlen) != 0) break;
	if(it == NULL){ cerr << "Erreur socket." << endl; exit(-1); }

	flux << "coucou !" << endl;

	// fclose(f);

	return 0;
}
