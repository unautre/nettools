#include <iostream>
#include <string>
#include <ext/stdio_filebuf.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include "webquest.hpp"

using namespace std;

URL::URL(): protocol(""), hostname(""), path("/"), port(-1) {}

URL::URL(const string& s): URL() {
	/* Finite State Automata
	[ http :// ] host [:port] [/path]
	  ^-0  123   ^-4   5 6      7	
	TODO: add error handling. */
	int etat = 0;
	for(int i = 0;i<(ssize_t)s.size();i++){
		switch(etat){
			case 0: if(s[i] == ':') etat = 1;
				else if(s[i] == '.'){ std::swap(protocol, hostname); etat = 4; hostname += s[i]; }
				else protocol += s[i]; break;
			case 1: if(isdigit(s[i])){ std::swap(hostname, protocol);
					port = s[i] - '0'; etat = 6; break; }
			case 2: if(s[i] == '/') etat++;
				break;
			case 3: etat = 4; hostname += s[i];
				break;
			case 4: if(s[i] == ':') etat = 5;
				else if(s[i] == '/'){ path = s.substr(i); i = s.size(); break; }
				else hostname += s[i];
				break;
			case 5: if(isdigit(s[i])){ port = s[i] - '0'; etat = 6; }
				break;
			case 6: if(isdigit(s[i])) port = (port * 10) + s[i] - '0';
				else{ path = s.substr(i); i = s.size(); }
				break;
		}
	}
}

URL::URL(const URL& o): protocol(o.protocol), hostname(o.hostname), path(o.path), port(o.port) {}

URL::operator string() const {
	string ret = "";
	if(protocol != "") ret += protocol + "://";
	ret += hostname;
	if(port > 0) ret += ":" + to_string(port);
	if(path == "" || path[0] != '/') ret += '/';
	ret += path;

	return ret;
}

string URL::getHost(void) const { return hostname; }
string URL::getProto(void) const { return protocol; }
string URL::getPath(void) const { return path; }
int URL::getPort(void) const { return port; }

ostream& operator<<(ostream& out, const URL& u){
	return out << (string)u;
}

int URL::getSock(){
	struct addrinfo *res, *it;
	int sock, ret;
	string proto = 	(port > 0) ? to_string(port) :
			(protocol != "") ? protocol :
			"http";

	if((ret = getaddrinfo(hostname.c_str(), proto.c_str(), NULL, &res))){
		cerr << "getaddrinfo: " << gai_strerror(ret) << endl;
		return -1; // TODO: raise exception.
	}

	for(it = res; it != NULL; it = it->ai_next){
		sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
		if(sock == -1) continue;
		if(connect(sock, it->ai_addr, it->ai_addrlen) == 0) break;
		close(sock);
	}

	if(it == NULL){ cerr << "Couldn't connect." << endl; return -1; }

	freeaddrinfo(res);
	return sock;
}

std::string URL::query(){
	return "";
}

int main(int argc, char **argv){
	if(argc <= 1) return 0;
	URL u(argv[1]);
	int sock = u.getSock();
	string s;
	__gnu_cxx::stdio_filebuf<char> filebuf(sock, std::ios::in | std::ios::out);
	iostream f(&filebuf);

	f << "GET " << u.getPath() << " HTTP/1.1" << endl;
	f << "Connection: close" << endl;
	f << "Host: " << u.getHost() << endl;
	f << endl;

	while(!f.eof()){
		getline(f, s);
		cout << s << endl;
	}

	return 0;
}
