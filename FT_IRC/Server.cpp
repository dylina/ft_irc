#include "Server.hpp"

Server::Server(Socket serverSocket) : _serverSocket(serverSocket)
{
	this->_addr.sa_family = AF_INET;

	bind(
		this->_serverSocket.getFd(),
		(struct sockaddr *)&this->_addr,
		sizeof(this->_addr)
	);

	std::cout << GREEN;
	this->_serverSocket.printAll();
	std::cout << RESET;
}
