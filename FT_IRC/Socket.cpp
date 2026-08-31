#include "Socket.hpp"
#include "colors.hpp"

const char* Socket::ErrorOfSocket::what() const throw()
{
	return "No socket";
}
const char* Socket::NoIp::what() const throw()
{
	return "No IP";
}
const char* Socket::NoPort::what() const throw()
{
	return "No port";
}

static bool portIsGood(std::string portRc)
{
	for (size_t i = 0; i < portRc.length(); i++)
	{
		if (isalpha(portRc[i]))
			return false;
	}
	return true;
}


// check errors in parameters of Socket(int domain, std::string ip, std::string port)
static void checkError(int domain, std::string ip, std::string portRc)
{
	if (ip.empty())
		throw Socket::NoIp();
	if (!portIsGood(portRc))
		throw Socket::NoPort();
	int port = atoi(portRc.c_str());
	if (port <= 0 || port > 65535)
		throw Socket::NoPort();

	if	(	   domain != AF_INET		//ipv4 protocol
			and domain != AF_INET6		//ipv6 protocol
			and domain != AF_UNIX		//Communication locale
			and domain != AF_LOCAL		//Alias de AF_UNIX sur les systèmes concernés
		)
		throw Socket::ErrorOfSocket();
}


//defaut constructor
Socket::Socket(int domain, std::string ip, std::string port)
{
	// check for errors in constructor
	try
	{
		checkError(domain, ip, port);
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << "Error :" << e.what() << RESET << '\n';
		throw e.what();
	}

	std::cerr << BOLD BRIGHT_GREEN  << "	Valid Args\n"
							<< "Domain : "; if(domain == AF_INET){std::cerr << "ipv4\n";}else if (domain == AF_INET6){std::cerr << "ipv6\n";}else if (domain == AF_UNIX){std::cerr << "local\n";}else{std::cerr << domain << '\n';} std::cerr
							<< "Ip : " << ip << '\n'
							<< "Port : " << port << '\n' << RESET << std::endl;

	// atribute the good domain
	this->sockDomain = domain;

	// it's the flux type for TCP uses
	this->sockType = SOCK_STREAM;

	//Standard protocol 0 = TCP
	this->sockProtocol = 0;

	//create a socket FD
	this->sockFd = socket(domain, this->sockType, this->sockProtocol);

	//si socket echoue on check l'erreur
	if (this->sockFd < 0)
	{
		perror("socket");
		throw Socket::ErrorOfSocket();
	}
	std::cerr << BOLD BRIGHT_GREEN << "socket() success!" << RESET << std::endl;

	this->port = port;
	this->ip = ip;

	this->adress.sin_family = this->sockDomain;
	this->adress.sin_port = htons(atoi(this->port.c_str()));
	this->adress.sin_addr.s_addr = INADDR_ANY;
}

struct sockaddr_in &Socket::getSocket()
{
	return this->adress;
}

int Socket::getDomain() const
{
	return this->sockDomain;
}

int Socket::getType() const
{
	return this->sockType;
}

int Socket::getProtocol() const
{
	return this->sockProtocol;
}

int Socket::getFd() const
{
	return this->sockFd;
}

//test functions
void Socket::printAll()
{
	std::cout << "## Informations of Socket ##" << std::endl;

	std::cout << "\n variable structures :\n" << std::endl;
	std::cout << "socketDomain:" << this->sockDomain << std::endl;
	std::cout << "socketProtocol:" << this->sockProtocol << std::endl;
	std::cout << "socketType:" << this->sockType << std::endl;
	std::cout << "sockFd:" << this->sockFd << std::endl;
	std::cout << "this ip: " << this->ip << std::endl;
	std::cout << "this port: " << this->port << std::endl;

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	std::cout << "len: " << len << std::endl;

}

Socket::~Socket()
{
	close(this->sockFd);
}
