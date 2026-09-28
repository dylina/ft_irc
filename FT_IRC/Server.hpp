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


class Server
{
	public:

    Server(int domain, std::string ip, std::string port, std::string password);
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
		bool isvalidNick(const std::string& nick) const;
		void addNewClient(int clientFd, sockaddr_in sockAddrClient);
		void dataReceived(std::vector<struct pollfd> &fds, int i);
		void processCommands(Client* client);
		void handlePASS(Client* client, std::vector<std::string>& params);
		void handleNICK(Client* client, std::vector<std::string>& params);
		void handleUSER(Client* client, std::vector<std::string>& params, std::string trailing);
		void finalizeRegistration(Client* client);

		Socket _serverSocket;
		std::map<int, Client*> clients;
		std::vector<struct pollfd> fds;
		std::string _password;
};





