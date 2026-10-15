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
	if (listen(this->_serverSocket.getFd(), SOMAXCONN) == -1)
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

	while (g_running)
	{
		if (g_sigusr1)
		{
			this->printAllClients();
			g_sigusr1 = 0;
		}

		for (size_t i = 1; i < this->fds.size(); ++i)
		{
			std::map<int, Client *>::iterator cit = this->clients.find(fds[i].fd);
			if (cit != this->clients.end() && cit->second->hasPendingOutput())
				fds[i].events = POLLIN | POLLOUT;
			else
				fds[i].events = POLLIN;
		}

		int retPoll = poll(&this->fds[0], this->fds.size(), -1);
		if (retPoll == -1)
		{
			if (!g_running)
				break;
			if (errno == EINTR)
				continue;
			std::cerr << BOLD BRIGHT_RED << "Error in poll() loop" << RESET << std::endl;
			break;
		}

		for (int i = static_cast<int>(this->fds.size()) - 1; i >= 0; --i)
		{
			if (fds[i].fd == this->_serverSocket.getFd())
			{
				if (fds[i].revents & POLLIN)
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
				continue;
			}

			if ((fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) && !(fds[i].revents & POLLIN))
			{
				int fd = fds[i].fd;
				std::map<int, Client *>::iterator cit = this->clients.find(fd);
				if (cit != this->clients.end())
					this->broadcastQuit(cit->second, "Connection closed");
				this->removeClient(fd);
				continue;
			}

			if (fds[i].revents & POLLIN)
			{
				this->dataReceived(fds, i); // poate sterge fds[i]
				continue;
			}

			if (fds[i].revents & POLLOUT)
			{
				std::map<int, Client *>::iterator cit = this->clients.find(fds[i].fd);
				if (cit != this->clients.end())
					cit->second->flushSendBuffer();
			}
		}
	}
	this->clean();
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
	int fd = fds[i].fd;

	std::map<int, Client *>::iterator it = this->clients.find(fd);
	if (it == this->clients.end())
	{
		this->removeClient(fd);
		return;
	}
	Client *client = it->second;

	int retRecv = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (retRecv > 0)
	{
		std::string data(buffer, retRecv);
		std::cerr << retRecv << " bytes received from " << fd << " fd" << std::endl;
		client->addData(data, client->getFdWithColor());
		this->processCommands(client); // apres quit client invalid
	}
	else
	{
		this->broadcastQuit(client, "Connection closed");
		this->removeClient(fd);
	}
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
		if ((*it).second->getNickName() == nick)
		{
			client->sendReply(":ircserv 433 * " + nick + " :Nickname is already in use");
			return;
		}
	}
	client->setNickName(nick);
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
		client->sendReply(":ircserv 462 " + client->getNickName() + " :You may not reregister");
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
		client->sendReply(":ircserv 001 " + client->getNickName() + " :Welcome to the IRC network " + client->getRealname() + "!" + client->getUsername() + "@localhost");
		client->sendReply(":ircserv 002 " + client->getNickName() + " :Your host is ircserv, running version 1.0");
		client->sendReply(":ircserv 003 " + client->getNickName() + " :This server was created today");
		client->sendReply(":ircserv 004 " + client->getNickName() + " ircserv 1.0 o o");
	}
}

void Server::processCommands(Client *client)
{
	while (client->hasCommands())
	{
		Client::parsedCommand cmd = client->popCommand();

		if (cmd.command == "CAP" || cmd.command == "PONG")
			continue;
		else if (cmd.command == "PASS")
			handlePASS(client, cmd.parametresMiddle);
		else if (cmd.command == "NICK")
			handleNICK(client, cmd.parametresMiddle);
		else if (cmd.command == "USER")
			handleUSER(client, cmd.parametresMiddle, cmd.trailing);
		else if (!client->isRegistered())
			client->sendReply(":ircserv 451 * :You have not registered");
		else if (cmd.command == "JOIN")
			cmdJoin(client, cmd);
		else if (cmd.command == "PART")
			cmdPart(client, cmd);
		else if (cmd.command == "TOPIC")
			cmdTopic(client, cmd);
		else if (cmd.command == "KICK")
			cmdKick(client, cmd);
		else if (cmd.command == "INVITE")
			cmdInvite(client, cmd);
		else if (cmd.command == "MODE")
			cmdMode(client, cmd);
		else if (cmd.command == "PRIVMSG")
			cmdPrivmsg(client, cmd);
		// else if (cmd.command == "NOTICE")
		// cmdNotice(client, cmd);
		else if (cmd.command == "PING")
			cmdPing(client, cmd);
		else if (cmd.command == "QUIT")
		{
			cmdQuit(client, cmd);
			return;
		}
		else
			client->sendReply(":ircserv 421 * " + cmd.command + " :Unknown command");
	}
}

void Server::clean()
{
	while (!this->clients.empty())
	{
		std::map<int, Client *>::iterator it = this->clients.begin();
		close(it->first);
		delete it->second;
		this->clients.erase(it);
	}
	this->fds.clear();
}

void Server::printAllClients()
{
	if (this->clients.empty())
		std::cerr << "No clients connected" << std::endl;
	for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
	{
		(*it).second->getAllInfo();
	}
}

void Server::cmdJoin(Client *client, Client::parsedCommand &cmd)
{
	if (cmd.parametresMiddle.empty())
	{
		client->sendReply(":ircserv 461 " + client->getNickName() + " JOIN :Not enough parameters");
		return;
	}

	// separation du param par virgule
	std::stringstream ss(cmd.parametresMiddle[0]);
	std::string chan;
	while (std::getline(ss, chan, ','))
	{
		// le nom commence tjr avec #
		if (chan.empty() || chan[0] != '#')
		{
			client->sendReply(":ircserv 476 " + client->getNickName() + " " + chan + " :Bad Channel Mask");
			continue;
		}

		// client deja dans le canal on l'ignore
		if (client->isInChannel(chan))
			continue;

		client->joinChannel(chan);

		// msg de JOIN (IRC: :nick JOIN :#canal)
		std::string joinMsg = ":" + client->getNickName() + " JOIN :" + chan;

		// envoier a tous meme le new client
		for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
			if (it->second->isInChannel(chan))
				it->second->sendReply(joinMsg);
	}
}

void Server::cmdPart(Client *client, Client::parsedCommand &cmd)
{
	if (cmd.parametresMiddle.empty())
	{
		client->sendReply(":ircserv 461 " + client->getNickName() + " PART :Not enough parameters");
		return;
	}

	std::stringstream ss(cmd.parametresMiddle[0]);
	std::string chan;

	while (std::getline(ss, chan, ','))
	{
		if (!client->isInChannel(chan))
		{
			client->sendReply(":ircserv 442 " + client->getNickName() + " " + chan + " :You're not on that channel");
			continue;
		}

		// motif pour partir apres les :
		std::string reason = cmd.trailing.empty() ? "" : (" :" + cmd.trailing);
		std::string partMsg = ":" + client->getNickName() + " PART " + chan + reason;

		for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			if (it->second->isInChannel(chan))
				it->second->sendReply(partMsg);
		}
		client->leaveChannel(chan);
	}
}

void Server::cmdTopic(Client *client, Client::parsedCommand &cmd)
{
	if (cmd.parametresMiddle.empty())
	{
		client->sendReply(":ircserv 461 " + client->getNickName() + " TOPIC :Not enough parameters");
		return;
	}

	std::string channel = cmd.parametresMiddle[0];

	if (!client->isInChannel(channel))
	{
		client->sendReply(":ircserv 442 " + client->getNickName() + " " + channel + " :You're not on that channel");
		return;
	}

	if (cmd.trailing.empty())
	{
		std::map<std::string, std::string>::iterator it =
			_channelTopics.find(channel);

		if (it == _channelTopics.end() || it->second.empty())
		{
			client->sendReply(":ircserv 331 " + client->getNickName() + " " + channel + " :No topic is set");
		}
		else
		{
			client->sendReply(":ircserv 332 " + client->getNickName() + " " + channel + " :" + it->second);
		}
		return;
	}

	_channelTopics[channel] = cmd.trailing;

	std::string topicMsg = ":" + client->getNickName() + " TOPIC " + channel + " :" + cmd.trailing;

	for (std::map<int, Client *>::iterator it = clients.begin();
		 it != clients.end(); ++it)
	{
		if (it->second->isInChannel(channel))
			it->second->sendReply(topicMsg);
	}
}

void Server::cmdKick(Client *client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdInvite(Client *client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdMode(Client *client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdPrivmsg(Client *client, Client::parsedCommand &cmd)
{

	if (cmd.parametresMiddle.empty())
	{
		client->sendReply(":ircserv 411 " + client->getNickName() + " :No recipient given (PRIVMSG)");
		return;
	}

	if (cmd.trailing.empty())
	{
		client->sendReply(":ircserv 412 " + client->getNickName() + " :No text to send");
		return;
	}

	std::string target = cmd.parametresMiddle[0];
	std::string msg = ":" + client->getNickName() + " PRIVMSG " + target + " :" + cmd.trailing;

	if (target[0] == '#')
	{
		if (!client->isInChannel(target))
		{
			client->sendReply(":ircserv 442 " + client->getNickName() + " " + target + " :You're not on that channel");
			return;
		}
		// envoi a tous sauf expediteur
		for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
		{
			if (it->second != client && it->second->isInChannel(target))
				it->second->sendReply(msg);
		}
	}
	else
	{
		// message prive a un nick
		for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
		{
			if (it->second->getNickName() == target)
			{
				it->second->sendReply(msg);
				return;
			}
		}
		client->sendReply(":ircserv 401 " + client->getNickName() + " " + target + " :No such nick");
	}
}

void Server::cmdPing(Client *client, Client::parsedCommand &cmd)
{
	// PING doit avoir un param.
	std::string token;

	if (!cmd.trailing.empty())
		token = cmd.trailing;
	else if (!cmd.parametresMiddle.empty())
		token = cmd.parametresMiddle[0];
	else
	{
		client->sendReply(":ircserv 461 * PING :Not enough parameters");
		return;
	}
	// send le pong et garde le token recu
	client->sendReply(":ircserv PONG ircserv :" + token);
}

void Server::broadcastQuit(Client *client, const std::string &reason)
{
	std::string quitMsg = ":" + client->getNickName() + " QUIT :" + reason;
	std::set<std::string> channels = client->getChannels();

	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		Client *other = it->second;
		if (other == client)
			continue;
		for (std::set<std::string>::iterator ch = channels.begin(); ch != channels.end(); ++ch)
		{
			if (other->isInChannel(*ch))
			{
				other->sendReply(quitMsg);
				break;
			}
		}
	}
}

void Server::removeClient(int fd)
{
	std::map<int, Client *>::iterator it = this->clients.find(fd);
	if (it != this->clients.end())
	{
		delete it->second;
		this->clients.erase(it);
	}
	for (std::vector<struct pollfd>::iterator p = this->fds.begin(); p != this->fds.end(); ++p)
	{
		if (p->fd == fd)
		{
			this->fds.erase(p);
			break;
		}
	}
	close(fd);
	std::cerr << "client fd [" << fd << "] disconnected" << std::endl;
}

void Server::cmdQuit(Client *client, Client::parsedCommand &cmd)
{
	std::string reason = cmd.trailing.empty() ? "Client Quit" : cmd.trailing;
	this->broadcastQuit(client, reason);
	this->removeClient(client->getFd()); // apres ca le `client` est invalide
}