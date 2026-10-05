#include "Server.hpp"
#include "Client.hpp"

const char *Server::BindErr::what() const throw()
{
	return "bind() error";
}

const char *Server::ListenErr::what() const throw()
{
	return "listen() error";
}

const char *Server::AcceptErr::what() const throw()
{
	return "accept() error";
}

const char *Server::RecvError::what() const throw()
{
	return "recv() error";
}

const char *Server::Clean::what() const throw()
{
	return "Server have data clen and close normally";
}

Server::Server(int domain, std::string ip, std::string port, std::string password)
	: _serverSocket(domain, ip, port), _password(password)
{

	if (bind(
			this->_serverSocket.getFd(),
			reinterpret_cast<const struct sockaddr *>(&this->_serverSocket.getSocket()),
			sizeof(this->_serverSocket.getSocket())) == -1)
	{
		perror("bind");
		throw BindErr();
	}
	std::cerr << BOLD BRIGHT_GREEN << "Server bind() success!" << RESET << std::endl;
}

void Server::on()
{
	if (listen(this->_serverSocket.getFd(), SOMAXCONN) == -1) // SOMAXCONN limit. max perm. sinon this->fds.size() = 0 error
	{
		perror("listen");
		throw ListenErr();
	}
	std::cout << "Server listening on " << ntohs(this->_serverSocket.getSocket().sin_port) << " .." << std::endl;

	struct pollfd server;
	server.fd = this->_serverSocket.getFd();
	server.events = POLLIN;
	server.revents = 0;

	this->fds.push_back(server);

	// Loop principale de poll qui check les fds en continue
	while (g_running)
	{
		if (g_sigusr1) // pour capter sigusr1 et print les clients connecter
		{
			this->printAllClients();
			g_sigusr1 = 0;
		}

		int retPoll = poll(&this->fds[0], this->fds.size(), -1);
		if (retPoll == -1 && g_running)
		{
			if (errno == EINTR) // ignore SIGUSR1
				continue;
			std::cerr << BOLD BRIGHT_RED << "Error in poll() loop" << RESET << std::endl;
			break;
		}

		for (size_t i = 0; i < this->fds.size(); ++i)
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

					try
					{
						this->addNewClient(clientFd, clientSocket);
					}
					catch (const std::exception &e)
					{
						std::cerr << BOLD BRIGHT_RED << e.what() << RESET << std::endl;
					}
				}
				else
				{
					try
					{
						this->dataReceived(fds, i);

					}
					catch (const std::exception &e)
					{
						std::cerr << BOLD BRIGHT_RED << e.what() << RESET << std::endl;
					}
				}
			}
			if (fds[i].revents & POLLOUT)
			{
				Client *client = this->clients[fds[i].fd];

				client->flushSendBuffer();

				if (!client->hasPendingOutput())
					fds[i].events &= ~POLLOUT;
			}
			if (!g_running)
			{
				this->clean();
			}
		}
	}
}

bool Server::isvalidNick(const std::string &nick) const
{
	if (nick.empty() || nick.length() > 9)
		return false;
	std::string validFirst = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]\\`^{}";
	if (validFirst.find(nick[0]) == std::string::npos)
		return false;
	std::string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_[]\\`^{}";
	for (size_t i = 1; i < nick.length(); ++i)
	{
		if (validChars.find(nick[i]) == std::string::npos)
			return false;
	}
	return true;
}

void Server::addNewClient(int clientFd, sockaddr_in sockAddrClient)
{
	if (clientFd < 0)
		throw AcceptErr();
	struct pollfd newClientStruct;

	newClientStruct.fd = clientFd;
	newClientStruct.events = POLLIN;
	newClientStruct.revents = 0;

	this->fds.push_back(newClientStruct);

	Client *newClientClass = new Client(sockAddrClient);
	newClientClass->setFD(clientFd);
	this->clients[newClientStruct.fd] = newClientClass;

	std::vector<struct pollfd>::iterator it = fds.end() - 1;
	std::cerr << GREEN << "New client add, fd: [" << (*it).fd << "], connected" << RESET << std::endl;
}

void Server::dataReceived(std::vector<struct pollfd> &fds, int i)
{
	char buffer[BUFFER_SIZE];

	int retRecv = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
	buffer[retRecv] = 0;
	if (retRecv > 0)
	{
		buffer[retRecv] = 0;
		std::string data(buffer, retRecv);
		std::cerr << retRecv << " bytes received from " << fds[i].fd << " fd" << std::endl;
		this->clients[fds[i].fd]->addData(data, this->clients[fds[i].fd]->getFdWithColor());
		this->processCommands(this->clients[fds[i].fd]);
		for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
			(*it).revents |= POLLOUT;
	}
	else if (retRecv == 0)
	{
		std::vector<struct pollfd>::iterator it = fds.begin() + i;
		std::cerr << "client fd [" << (*it).fd << "] disconnected" << std::endl;

		delete this->clients[(*it).fd];
		this->clients.erase((*it).fd);
		close((*it).fd);
		this->fds.erase(it);
	}
	else
		std::cerr << "Error client received in fd [" << fds[i].fd << "]" << std::endl;
}

void Server::handlePASS(Client *client, std::vector<std::string> &params)
{
	if (params.empty())
	{
		client->sendReply(":ircserv 461 * PASS :Not enough parameters");
		return;
	}
	if (params[0] != this->_password)
	{
		client->sendReply(":ircserv 464 * :Password incorrect");
		return;
	}
	client->setPassOk(true);
	finalizeRegistration(client);
}

void Server::handleNICK(Client *client, std::vector<std::string> &params)
{
	if (!client->isPassOk())
	{
		client->sendReply(":ircserv 464 * :Password required");
		return;
	}
	if (params.empty())
	{
		client->sendReply(":ircserv 431 * :No nickname given");
		return;
	}
	std::string nick = params[0];

	if (!isvalidNick(nick))
	{
		client->sendReply(":ircserv 432 * " + nick + " :Erroneous nickname");
		return;
	}
	std::map<int, Client *>::iterator it = this->clients.begin();
	for (; it != this->clients.end(); ++it)
	{
		if ((*it).second->getRealname() == nick)
		{
			client->sendReply(":ircserv 433 * " + nick + " :Nickname is already in use");
			return;
		}
	}
	client->setRealname(nick);
	client->setHasNick(true);
	finalizeRegistration(client);
}

void Server::handleUSER(Client *client, std::vector<std::string> &params, std::string trailing)
{
	if (!client->isPassOk())
	{
		client->sendReply(":ircserv 464 * :Password required");
		return;
	}
	if (client->isRegistered())
	{
		client->sendReply(":ircserv 462 " + client->getRealname() + " :You may not reregister");
		return;
	}

	std::string realname = trailing;
	if (realname.empty() && params.size() >= 4)
		realname = params[3];

	std::cerr << "realname stocké : [" << realname << "]" << std::endl;
	if (params.size() < 3 || realname.empty())
	{
		client->sendReply(":ircserv 461 * USER :Not enough parameters");
		return;
	}
	client->setUsername(params[0]);
	client->setRealname(realname);
	client->setHasUser(true);
	finalizeRegistration(client);
}

void Server::finalizeRegistration(Client *client)
{
	if (client->isPassOk() && client->hasNick() && client->hasUser())
	{
		client->setRegistered(true);
		client->sendReply(":ircserv 001 " + client->getRealname() + " :Welcome to the IRC network " + client->getRealname() + "!" + client->getUsername() + "@localhost");
		client->sendReply(":ircserv 002 " + client->getRealname() + " :Your host is ircserv, running version 1.0");
		client->sendReply(":ircserv 003 " + client->getRealname() + " :This server was created today");
		client->sendReply(":ircserv 004 " + client->getRealname() + " ircserv 1.0 o o");
	}
}

void Server::processCommands(Client *client)
{
	while (client->hasCommands())
	{
		Client::parsedCommand cmd = client->popCommand();

		if (cmd.command == "PASS")
			handlePASS(client, cmd.parametresMiddle);
		else if (cmd.command == "NICK")
			handleNICK(client, cmd.parametresMiddle);
		else if (cmd.command == "USER")
			handleUSER(client, cmd.parametresMiddle, cmd.trailing);
		else if (!client->isRegistered())
			client->sendReply(":ircserv 451 * :You have not registered");
		// else if (cmd.command  == "JOIN")
		// 	cmdJoin(client, cmd);				-> to do
		// else if (cmd.command  == "PART")
		// 	cmdPart(client, cmd);				-> to do
		// else if (cmd.command  == "TOPIC")
		// 	cmdTopic(client, cmd);				-> to do
		// else if (cmd.command  == "KICK")
		// 	cmdKick(client, cmd);				-> to do
		// else if (cmd.command  == "INVITE")
		// 	cmdInvite(client, cmd);				-> to do
		// else if (cmd.command  == "MODE")
		// 	cmdMode(client, cmd);				-> to do
		// else if (cmd.command  == "PRIVMSG")
		// 	cmdPrivmsg(client, cmd);				-> to do
		// else if (cmd.command  == "NOTICE")
		// 	cmdNotice(client, cmd);				-> to do
		// else if (cmd.command  == "PING")
		// 	cmdPing(client, cmd);				-> to do
		// else if (cmd.command  == "QUIT")
		// 	cmdQuit(client, cmd);				-> to do
		else
			client->sendReply(":ircserv 421 * " + cmd.command + " :Unknown command");
	}
}

void Server::clean()
{
	while (!this->clients.empty())
	{
		std::map<int, Client *>::iterator it = this->clients.begin();
		delete (*it).second;
		this->clients.erase(it);
	}
}

void Server::printAllClients()
{
	std::ofstream dump("/tmp/ircserv_dump");

	if (!dump.is_open())
		return;

	if (this->clients.empty())
	{
		dump << "No clients connected" << std::endl;
		return;
	}

	for (std::map<int, Client *>::iterator it = this->clients.begin();
		 it != this->clients.end(); ++it)
	{
		it->second->getAllInfo(dump);
	}
}
