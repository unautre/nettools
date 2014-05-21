#include <iostream>
#include <string>

class URL {
	std::string protocol, hostname, path;
	int port;
public:
	URL(); // constructeur vide
	URL(const std::string&); // à partir d'une chaine
	URL(const URL&); // constructeur de recopie

	operator std::string() const;
	std::string getHost(void) const;
	std::string getProto(void) const;
	std::string getPath(void) const;
	int getPort(void) const;

	friend std::ostream& operator<<(std::ostream&, const URL&);
};

using namespace std;

URL::URL(): protocol(""), hostname(""), path("/"), port(-1) {}

URL::URL(const string& s): URL() {
	/* Finite State Automata
	[ http :// ] host [:port] [/path]
	  ^-0  123   ^-4   5 6      7	
	TODO: add error handling. */
	int etat = 0;
	for(int i = 0;i<s.size();i++){
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

int main(void){
	string s;

	do{
		cin >> s;
		URL u(s);
		cout << u << endl;
	}while(s != "" && cin);

	return 0;
}
