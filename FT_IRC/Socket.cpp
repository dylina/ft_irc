#include "Socket.hpp"

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




// utils functions
static void checkError(int domain, std::string ip, int port)
{
	if (ip.empty())
		throw Socket::NoIp();

	if (port <= 0 || port > 65535)
		throw Socket::NoPort();

	if	(	   domain != AF_INET		//ipv4
			and domain != AF_INET6		//ipv6
			and domain != AF_UNIX		//Communication locale
			and domain != AF_LOCAL		//Alias de AF_UNIX sur les systèmes concernés
		)
		throw Socket::ErrorOfSocket();
}






//defaut constructor
Socket::Socket(int domain, std::string ip, int port)
{

	// check for errors in constructor
	try
	{
		checkError(domain, ip, port);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error :" << e.what() << '\n';
		throw e.what();
	}

	this->socketDomain = domain;	// atribute the good domain

	this->socketType = SOCK_STREAM;	// it's the flux type

	this->socketProtocol = 0;		//Standard protocol = TCP

	this->sockFd = socket(domain, this->socketType, this->socketProtocol);	//create a socket FD
	if (this->sockFd < 0)
	{
		perror("socket");
		throw Socket::ErrorOfSocket();
	} //si socket echoue on regarde l'erreur

	memset(&hints, 0, sizeof(hints));				// empty all hints struct

	this->hints.ai_family = domain;
	this->hints.ai_socktype = SOCK_STREAM;
	this->hints.ai_protocol = 0;

	// use stringstream for translate number on string
	std::stringstream portToString;
	portToString << port;
	std::stringstream IpToString;
	IpToString << ip;

	// save the parameters
	this->port = portToString.str();
	this->ip = IpToString.str();

	//Translate name of a service location and/or a service name to set of socket addresses.
	int errorGetAddrInfo = getaddrinfo(this->ip.c_str(), this->port.c_str(), &hints, &result);

	if (errorGetAddrInfo != 0)
	{
		std::cerr << "Error with getaddrinfo" << std::endl;

	}



	//test
	printAll();
}








//test functions
void Socket::printAll()
{
	std::cout << "## Informations ##" << std::endl;

	std::cout << "\n# this->hints. :" << std::endl;
	std::cout << "ai_addr: " << this->hints.ai_addr << std::endl;
	std::cout << "ai_addrlen: " << this->hints.ai_addrlen << std::endl;
	std::cout << "ai_canonname: " << static_cast<void *>(this->hints.ai_canonname) << std::endl;
	std::cout << "ai_family: " << this->hints.ai_family << std::endl;
	std::cout << "hints.ai_flags: " << this->hints.ai_flags << std::endl;
	std::cout << "hints.ai_next: " << this->hints.ai_next << std::endl;
	std::cout << "hints.ai_protocol: " << this->hints.ai_protocol << std::endl;
	std::cout << "hints.ai_socktype: " << this->hints.ai_socktype << std::endl;

		if (result)
	{
		std::cout << "ai_family: " << result->ai_family << std::endl;
		std::cout << "ai_socktype: " << result->ai_socktype << std::endl;
		std::cout << "ai_protocol: " << result->ai_protocol << std::endl;
		std::cout << "ai_addr: " << result->ai_addr << std::endl;
		std::cout << "ai_addrlen: " << result->ai_addrlen << std::endl;
		std::cout << "ai_next: " << result->ai_next << std::endl;
	}

	std::cout << "\n variable structures :\n" << std::endl;
	std::cout << "result:" << this->result << std::endl;
	std::cout << "socketDomain:" << this->socketDomain << std::endl;
	std::cout << "socketProtocol:" << this->socketProtocol << std::endl;
	std::cout << "socketType:" << this->socketType << std::endl;
	std::cout << "sockFd:" << this->sockFd << std::endl;

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

}


