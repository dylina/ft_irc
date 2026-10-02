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
	Server(int domain, std::string ip, std::string port);

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

private:
	// mount a struct pollfd for new client and add in variables server
	void addNewClient(int clientFd);
	// catch message from a already connected client
	void dataReceived(std::vector<struct pollfd> &fds, int i);

	void executeCommands(Client &client, Client::parsedCommand &cmd);

	void cmdJoin(Client &client, Client::parsedCommand &cmd);
	void cmdPart(Client &client, Client::parsedCommand &cmd);
	void cmdPrivmsg(Client &client, Client::parsedCommand &cmd);
	void cmdTopic(Client &client, Client::parsedCommand &cmd);
	void cmdKick(Client &client, Client::parsedCommand &cmd);
	void cmdInvite(Client &client, Client::parsedCommand &cmd);
	void cmdQuit(Client &client, Client::parsedCommand &cmd);

	// au cas ou
	void cmdMode(Client &client, Client::parsedCommand &cmd);
	void cmdPass(Client &client, Client::parsedCommand &cmd);
	void cmdNick(Client &client, Client::parsedCommand &cmd);
	void cmdUser(Client &client, Client::parsedCommand &cmd);

	Socket _serverSocket;
	// tableau associatif de pairs, [int Clientfd] -> Client User,  !a delete a la fin!
	std::map<int, Client *> clients;
	// tableau dynamique de surveillance de fd pour poll()
	std::vector<struct pollfd> fds;
};
