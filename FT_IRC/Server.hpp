#pragma once
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "Socket.hpp"

class Server
{
	public:

		Server(int domain, std::string ip, std::string port);

		void on();

		class BindErr : public std::exception
		{
			public:
				const char* what() const throw();
		};
		class ListenErr : public std::exception
		{
			public:
				const char* what() const throw();
		};
		class AcceptErr : public std::exception
		{
			public:
				const char* what() const throw();
		};

	private:

		Socket _serverSocket;
		struct sockaddr_in _addr;
};
