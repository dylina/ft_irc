#include "Server.hpp"

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

Server::Server(int domain, std::string ip, std::string port) : _serverSocket(domain, ip, port)
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
	while (true)
	{
		// avant chaque poll(): on active POLLOUT seulement si le client a des donnees en attente d'envoi
		for (size_t i = 0; i < this->fds.size(); i++)
		{
			if (fds[i].fd == this->_serverSocket.getFd())
				continue; // le socket serv n'envoie jamais rien
			std::map<int, Client *>::iterator cit = this->clients.find(fds[i].fd);
			if (cit != this->clients.end() && cit->second->hasPendingOutput())
				fds[i].events = POLLIN | POLLOUT;
			else
				fds[i].events = POLLIN;
		}

		int retPoll = poll(&this->fds[0], this->fds.size(), -1);
		if (retPoll == -1)
		{
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
						this->addNewClient(clientFd);
					}
					catch (const std::exception &e)
					{
						std::cerr << BOLD BRIGHT_RED << e.what() << RESET << std::endl;
					}
					continue;
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
			// POLLOUT : le fd est ptre a recevoir des dones en ecriture
			if (i < fds.size() && (fds[i].revents & POLLOUT))
			{
				std::map<int, Client *>::iterator cit = this->clients.find(fds[i].fd);
				if (cit != this->clients.end())
					cit->second->flushSendBuffer();
			}
		}
	}
}

void Server::addNewClient(int clientFd)
{
	if (clientFd < 0)
		throw AcceptErr();
	struct pollfd newClientStruct;

	newClientStruct.fd = clientFd;
	newClientStruct.events = POLLIN;
	newClientStruct.revents = 0;

	this->fds.push_back(newClientStruct);

	Client *newClientClass = new Client;
	this->clients[newClientStruct.fd] = newClientClass;

	std::vector<struct pollfd>::iterator it = fds.end() - 1;
	std::cerr << GREEN << "New client add, fd: [" << (*it).fd << "], connected" << RESET << std::endl;
}

void Server::dataReceived(std::vector<struct pollfd> &fds, int i)
{
	char buffer[BUFFER_SIZE + 1]; //+1 si le retRecv == 512 et buffer[512] est overflow

	int retRecv = recv(fds[i].fd, buffer, sizeof(buffer), 0);

	// client data received
	if (retRecv > 0)
	{
		buffer[retRecv] = 0;
		std::string data(buffer, retRecv);
		std::cerr << retRecv << " bytes received from " << fds[i].fd << " fd" << std::endl;

		Client *client = this->clients[fds[i].fd]; // prends le *
		client->addData(data, fds[i].fd);		   // ajoute le data dans buff et parse les lignes

		while (client->hasPendingCommands()) // vide le derniere
		{
			Client::parsedCommand cmd = client->popCommand(); // enleve la premiere cmd
			this->executeCommands(*client, cmd);			  // envoi au bon handler
		}
	}
	// client lost
	else if (retRecv == 0)
	{
		std::vector<struct pollfd>::iterator it = fds.begin() + i;
		std::cerr << "client fd [" << (*it).fd << "] disconnected" << std::endl;

		this->clients.erase((*it).fd);
		close((*it).fd);
		this->fds.erase(it);
	}
	// error
	else
		std::cerr << "Error client received in fd [" << fds[i].fd << "]" << std::endl;
}

void Server::executeCommands(Client &client, Client::parsedCommand &cmd)
{
	std::string upper = cmd.command;
	for (size_t i = 0; i < upper.size(); i++)
		upper[i] = std::toupper(static_cast<unsigned char>(upper[i]));

	if (upper.empty())
		return;

	if (!client.isRegistered() && upper != "PASS" && upper != "NICK" && upper != "USER")
	{

		client.queueMessage(":ircserv 451 * :You have not registered\r\n");
		return;
	}

	if (upper == "PASS")
		cmdPass(client, cmd);
	else if (upper == "NICK")
		cmdNick(client, cmd);
	else if (upper == "USER")
		cmdUser(client, cmd);
	else if (upper == "JOIN")
		cmdJoin(client, cmd);
	else if (upper == "PART")
		cmdPart(client, cmd);
	else if (upper == "TOPIC")
		cmdTopic(client, cmd);
	else if (upper == "KICK")
		cmdKick(client, cmd);
	else if (upper == "INVITE")
		cmdInvite(client, cmd);
	else if (upper == "MODE")
		cmdMode(client, cmd);
	else if (upper == "PRIVMSG")
		cmdPrivmsg(client, cmd);
	else if (upper == "QUIT")
		cmdQuit(client, cmd);
	else
		client.queueMessage(":ircserv 421 " + client.getNickName() + " " + cmd.command + " :Unknown command\r\n");
}

void Server::cmdJoin(Client &client, Client::parsedCommand &cmd)
{
	if (cmd.parametresMiddle.empty())
	{
		client.queueMessage(":ircserv 461 " + client.getNickName() + " JOIN :Not enough parameters\r\n");
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
			client.queueMessage(":ircserv 476 " + client.getNickName() + " " + chan + " :Bad Channel Mask\r\n");
			continue;
		}

		// client deja dans le canal on l'ignore
		if (client.isInChannel(chan))
			continue;

		client.joinChannel(chan);

		// msg de JOIN (IRC: :nick JOIN :#canal)
		std::string joinMsg = ":" + client.getNickName() + " JOIN :" + chan + "\r\n";

		// envoier a tous meme le new client
		for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			if (it->second->isInChannel(chan))
				it->second->queueMessage(joinMsg);
		}
	}
}

void Server::cmdPart(Client &client, Client::parsedCommand &cmd)
{
	if (cmd.parametresMiddle.empty())
	{
		client.queueMessage(":ircserv 461 " + client.getNickName() + " PART :Not enough parameters\r\n");
		return;
	}

	std::stringstream ss(cmd.parametresMiddle[0]);
	std::string chan;

	while (std::getline(ss, chan, ','))
	{
		if (!client.isInChannel(chan))
		{
			client.queueMessage(":ircserv 442 " + client.getNickName() + " " + chan + " :You're not on that channel\r\n");
			continue;
		}

		// motif pour partir apres les :
		std::string reason = cmd.trailing.empty() ? "" : (" :" + cmd.trailing);
		std::string partMsg = ":" + client.getNickName() + " PART " + chan + reason + "\r\n";

		for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			if (it->second->isInChannel(chan))
				it->second->queueMessage(partMsg);
		}
		client.leaveChannel(chan);
	}
}

void Server::cmdPrivmsg(Client &client, Client::parsedCommand &cmd)
{
	if (cmd.parametresMiddle.empty())
	{
		client.queueMessage(":ircserv 411 " + client.getNickName() + " :No recipient given (PRIVMSG)\r\n");
		return;
	}

	if (cmd.trailing.empty())
	{
		client.queueMessage(":ircserv 412 " + client.getNickName() + " :No text to send\r\n");
		return;
	}

	std::string target = cmd.parametresMiddle[0];
	std::string msg = ":" + client.getNickName() + " PRIVMSG " + target + " :" + cmd.trailing + "\r\n";

	if (target[0] == '#')
	{
		if (!client.isInChannel(target))
		{
			client.queueMessage(":ircserv 442 " + client.getNickName() + " " + target + " :You're not on that channel\r\n");
			return;
		}
		// envoi a tous sauf expediteur
		for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
		{
			if (it->second != &client && it->second->isInChannel(target))
				it->second->queueMessage(msg);
		}
	}
	else
	{
		// message prive a un nick
		for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
		{
			if (it->second->getNickName() == target)
			{
				it->second->queueMessage(msg);
				return;
			}
		}
		client.queueMessage(":ircserv 401 " + client.getNickName() + " " + target + " :No such nick\r\n");
	}
}
void Server::cmdTopic(Client &client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdKick(Client &client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdInvite(Client &client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdQuit(Client &client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdMode(Client &client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdPass(Client &client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdNick(Client &client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdUser(Client &client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}
