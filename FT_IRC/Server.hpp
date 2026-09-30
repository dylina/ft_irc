#pragma once

#include <cstddef>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <iterator>
#include <map>
#include <csignal>
#include <cerrno>

#include "Socket.hpp"
#include "colors.hpp"
#include "Client.hpp"

extern volatile sig_atomic_t g_running;
extern volatile sig_atomic_t g_sigusr1;


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
		class Clean : public std::exception
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

		void clean();

		// debug functions
		void printAllClients();

		//mount a struct pollfd for new client and add in variables server
		void addNewClient(int clientFd);

		//catch message from a already connected client
		void dataReceived(std::vector<struct pollfd> &fds, int i);


		void executeCommands(Client &client, Client::parsedCommand &cmd);

		// void cmdJoin(Client &client, Client::parsedCommand &cmd);
		// void cmdPart(Client &client, Client::parsedCommand &cmd);
		// void cmdPrivmsg(Client &client, Client::parsedCommand &cmd);
		// void cmdTopic(Client &client, Client::parsedCommand &cmd);
		// void cmdKick(Client &client, Client::parsedCommand &cmd);
		// void cmdInvite(Client &client, Client::parsedCommand &cmd);
		// void cmdQuit(Client &client, Client::parsedCommand &cmd);
		// void cmdPing(Client &client, Client::parsedCommand &cmd);

		// //au cas ou
		// void cmdNotice(Client &client, Client::parsedCommand &cmd);
		// void cmdMode(Client &client, Client::parsedCommand &cmd);
		// void cmdPass(Client &client, Client::parsedCommand &cmd);
		// void cmdNick(Client &client, Client::parsedCommand &cmd);
		// void cmdUser(Client &client, Client::parsedCommand &cmd);

		//tableau associatif de pairs, [int Clientfd] -> Client User,  !a delete a la fin!
		std::map<int,Client *> clients;
		//tableau dynamique de surveillance de fd pour poll()
		std::vector<struct pollfd> fds;
};
