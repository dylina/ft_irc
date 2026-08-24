#pragma once

#include <cstddef>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <iterator>
#include <map>

#include "Socket.hpp"
#include "colors.hpp"
#include "Client.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif


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
		class RecvError : public std::exception
		{
			public:
				const char* what() const throw();
		};


	private:

		void addNewClient(int clientFd);
		void dataReceived(std::vector<struct pollfd> &fds, int i);

		Socket _serverSocket;
		std::map<int,Client> clients;
		std::vector<struct pollfd> fds;
};
