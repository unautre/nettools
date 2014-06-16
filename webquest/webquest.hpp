#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

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

	// temporary, just for testing.
	int getSock();
	std::string query();
};

