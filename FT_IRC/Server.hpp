#pragma once

#include <cstddef>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <iterator>
#include <map>
#include <csignal>
#include <cerrno>
#include <fstream>
#include <string>

#include "Channel.hpp"
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
		const char *what() const throw();
	};
	class ListenErr : public std::exception
	{
	public:
		const char *what() const throw();
	};
	class AcceptErr : public std::exception
	{
	public:
		const char *what() const throw();
	};
	class RecvError : public std::exception
	{
	public:
		const char *what() const throw();
	};
	class Clean : public std::exception
	{
	public:
		const char *what() const throw();
	};

private:
	bool isvalidNick(const std::string &nick) const;
	void addNewClient(int clientFd, sockaddr_in sockAddrClient);
	void dataReceived(std::vector<struct pollfd> &fds, int i);
	void processCommands(Client *client);
	bool isNickIsAlreadyUse(Client *client, std::string &nick);



	void handlePASS(Client *client, std::vector<std::string> &params);
	void handleNICK(Client *client, std::vector<std::string> &params);
	void handleUSER(Client *client, std::vector<std::string> &params, std::string trailing);
	void finalizeRegistration(Client *client);

	void cmdJoin(Client *client, Client::parsedCommand &cmd);
	void cmdPart(Client *client, Client::parsedCommand &cmd);
	void cmdPrivmsg(Client *client, Client::parsedCommand &cmd);
	void cmdPing(Client *client, Client::parsedCommand &cmd);
	void cmdKick(Client *client, Client::parsedCommand &cmd);
	void cmdTopic(Client *client, Client::parsedCommand &cmd);
	void cmdInvite(Client *client, Client::parsedCommand &cmd);
	void cmdMode(Client *client, Client::parsedCommand &cmd);
	void cmdQuit(Client *client, Client::parsedCommand &cmd);

	Socket _serverSocket;
	std::map<int, Client *> clients;
	std::vector<struct pollfd> fds;
	std::string _password;
	std::map<std::string ,Channel *> _channels;

	// QUIT
	void broadcastQuit(Client *client, std::string reason);
	void removeClient(int index); // suprime le client en clients et en fds

	void clean();

	// debug functions
	void printAllClients();
};
