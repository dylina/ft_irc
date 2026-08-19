#include "Server.hpp"

const char* Server::BindErr::what() const throw()
{
	return "bind() error";
}

const char* Server::ListenErr::what() const throw()
{
	return "listen() error";
}

const char* Server::AcceptErr::what() const throw()
{
	return "accept() error";
}


Server::Server(int domain, std::string ip, std::string port) : _serverSocket(domain, ip, port), _addr()
{

	this->_addr.sin_family = AF_INET;
	this->_addr.sin_port = htons(atoi(port.c_str()));
    _addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (	bind(
			this->_serverSocket.getFd(),
			reinterpret_cast<const struct sockaddr*>(&this->_addr),
			sizeof(this->_addr)
			) == -1 )
	{
		perror("bind");
		throw BindErr();
	}

	std::cerr << "Server bind() success" << std::endl;
}

void Server::on()
{
	if (listen(this->_serverSocket.getFd(), 10) == -1)
	{
		perror("listen");
		throw ListenErr();
	}

	std::cout << "Server listening .." << std::endl;

	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);


	// test for client connect
		int clientFd = accept(
			this->_serverSocket.getFd(),
			reinterpret_cast<struct sockaddr *>(&clientAddr),
			&addrLen
		);

		if (clientFd == -1)
		{
			perror("accept");
			throw AcceptErr();
		}

		std::cout << clientFd << std::endl;
		close(clientFd);
	// end test --------------------
}

