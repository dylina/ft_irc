#include "Server.hpp"
#include "Socket.hpp"
#include "colors.hpp"
#include <cstddef>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>


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


Server::Server(int domain, std::string ip, std::string port) : _serverSocket(domain, ip, port)
{

	if (	bind(
			this->_serverSocket.getFd(),
			reinterpret_cast<const struct sockaddr*>(&this->_serverSocket.getSocket()),
			sizeof(this->_serverSocket.getSocket())
			) == -1 )
	{
		perror("bind");
		throw BindErr();
	}
	std::cerr << BOLDGREEN << "Server bind() success!" << RESET << std::endl;

}

void Server::on()
{
	if (listen(this->_serverSocket.getFd(), 10) == -1)
	{
		perror("listen");
		throw ListenErr();
	}
	std::cout << "Server listening on " << ntohs(this->_serverSocket.getSocket().sin_port) << " .." << std::endl;

	struct pollfd fds[10];
	fds[0].fd = this->_serverSocket.getFd();
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	// Loop principale de poll qui check les fds en continue
	while (true)
	{
		int ret = poll(fds, 10, -1);
		if (ret == -1)
		{	
			std::cerr << BOLDRED << "Error in poll() loop" << RESET << std::endl;
			break;
		}

		for (int i = 0; i < 10; ++i)
		{
			// connexion flags : POLLIN = Il y a des données en attente de lecture.
			if (fds[i].revents & POLLIN) 
			{
				if (fds[i].fd == this->_serverSocket.getFd())
				{
					sockaddr_in clientSocket;
					socklen_t clientLen = sizeof(clientSocket);

					std::cerr << "log: connexion of new client ..." << std::endl;
					int clientFd = accept(fds[i].fd, reinterpret_cast<struct sockaddr *>(&clientSocket), &clientLen);

					if (clientFd)
						std::cerr << GREEN << "New client [" << clientFd << "] connected" << RESET << std::endl;
					else
						std::cerr << RED << "!New client error received" << RESET << std::endl;
				}
			}
		}
	}

}

