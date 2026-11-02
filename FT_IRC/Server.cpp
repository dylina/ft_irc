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
		if (retPoll == -1 && g_running)
		{
			if (!g_running)
				break;
			if (errno == EINTR)
				continue;
			std::cerr << BOLD BRIGHT_RED << "Error in poll() loop" << RESET << std::endl;
			break;
		}

		for (size_t i = 0; i < this->fds.size(); ++i)
		{
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
				continue;
			}

			if (i < fds.size() && (fds[i].revents & POLLOUT))
			{
				std::map<int, Client *>::iterator cit = this->clients.find(fds[i].fd);
				if (cit != this->clients.end())
					cit->second->flushSendBuffer();
			}
			if (!g_running)
			{
				this->clean();
			}
		}
	}
}

static std::string formatMessage(
	int error,
	std::string msg,
	Client *client,
	std::string channelTarget = "")
{
	std::ostringstream oss;
	oss << ":ircserv "
		<< std::setfill('0') << std::setw(3) << error
		<< " " << client->getNickName();

	if (!channelTarget.empty())
		oss << " " << channelTarget;

	oss << " " << msg;

	return oss.str();
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
		client->sendReply(formatMessage(ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS, client, ""));
		return;
	}
	if (params[0] != this->_password)
	{
		client->sendReply(formatMessage(ERR_PASSWDMISMATCH, MSG_PASSWDMISMATCH, client, ""));
		return;
	}
	client->setPassOk(true);
	finalizeRegistration(client);
}

// check if nickname is arleady use by another client and return true if is
bool Server::isNickIsAlreadyUse(Client *client, std::string &nick)
{
	std::map<int, Client *>::iterator it = this->clients.begin();
	for (; it != this->clients.end(); ++it)
	{
		if ((*it).second->getNickName() == nick && (*it).first != client->getFd())
		{
			client->sendReply(formatMessage(ERR_NICKNAMEINUSE, MSG_NICKNAMEINUSE, client, ""));
			return true;
		}
	}
	return false;
}

void Server::handleNICK(Client *client, std::vector<std::string> &params)
{
	// if (!client->isPassOk())
	// {
	// 	client->sendReply(":ircserv 464 * :Password required");
	// 	return;
	// } pas en accord avec la norme hexchat pour le CAP LS ?
	if (params.empty())
	{
		client->sendReply(formatMessage(ERR_NONICKNAMEGIVEN, MSG_NONICKNAMEGIVEN, client, ""));
		return;
	}
	std::string nick = params[0];

	if (!isvalidNick(nick))
	{
		client->sendReply(":ircserv 432 * " + nick + " :Erroneous nickname");
		return;
	}

	if (isNickIsAlreadyUse(client, nick) == true)
		return;
	client->setNickName(nick);
	client->setHasNick(true);
	finalizeRegistration(client);
}

void Server::handleUSER(Client *client, std::vector<std::string> &params, std::string trailing)
{
	// if (!client->isPassOk())
	// {
	// 	client->sendReply(":ircserv 464 * :Password required");
	// 	return;
	// } pas en accord avec la norme hexchat pour le CAP LS ?
	if (client->isRegistered())
	{
		client->sendReply(":ircserv 462 " + client->getNickName() + " :You may not reregister");
		return;
	}

	std::string realname = trailing;
	if (realname.empty() && params.size() >= 4)
		realname = params[3];

	if (params.size() < 3 || realname.empty())
	{
		client->sendReply(":ircserv 461 * USER :Not enough parameters");
		return;
	}

	client->setUsername(params[0]);
	client->setHasUser(true);

	finalizeRegistration(client);
}

void Server::finalizeRegistration(Client *client)
{
	if (client->isPassOk() && client->hasNick() && client->hasUser())
	{
		client->setRegistered(true);
		client->sendReply(":ircserv 001 " + client->getNickname() + " :Welcome to the IRC network " + client->getNickname() + "!" + client->getUsername() + "@localhost");
		client->sendReply(":ircserv 002 " + client->getNickname() + " :Your host is ircserv, running version 1.0");
		client->sendReply(":ircserv 003 " + client->getNickname() + " :This server was created today");
		client->sendReply(":ircserv 004 " + client->getNickname() + " ircserv 1.0 o o");
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
		// else if (cmd.command == "PING")
		// 	cmdPing(client, cmd);
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
	while (!this->_channels.empty())
	{
		std::map<std::string, Channel *>::iterator it = this->_channels.begin();
		delete (*it).second;
		this->_channels.erase(it);
	}
	this->fds.clear();
}

void Server::printAllClients()
{
	std::ofstream dump("/tmp/ircserv_dump");

	if (!dump.is_open())
		return;

	if (this->clients.empty())
	{
		dump << "No clients connected" << std::endl;
	}
	else
	{
		for (std::map<int, Client *>::iterator it = this->clients.begin();
			 it != this->clients.end(); ++it)
		{
			it->second->getAllInfo(dump);
		}
	}
	if (this->_channels.empty())
		dump << "No channel" << std::endl;
	else
	{
		dump << "------------list channel--------------" << std::endl;
		for (std::map<std::string, Channel *>::iterator it = this->_channels.begin(); it != this->_channels.end(); it++)
		{
			dump << (*it).first.c_str() << "  Have " << (*it).second->getNbrConnected() << " users actives" << std::endl;
		}
		dump << "------------end channel--------------" << std::endl;
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
		if (!_channels.empty())
		{
			try
			{
				if (_channels.at(chan)->isInChannel(client))
					continue;
			}
			catch (...)
			{
			}
		}

		try
		{
			this->_channels.at(chan)->joinChannel(client);
		}
		catch (...) // si on catch une erreur c'est que le salon n'existe pas encore, alor on le creer
		{
			try
			{
				std::string noTopic;
				Channel *newChannel = new Channel(client, noTopic, chan);
				this->_channels[chan] = newChannel;
			}
			catch (...)
			{
			}
		}

		// msg de JOIN (IRC: :nick JOIN :#canal)
		std::string joinMsg = ":" + client->getNickName() + " JOIN :" + chan;

		// envoier a tous meme le new client
		this->_channels[chan]->sendToAll(joinMsg);
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
		if (chan.empty())
			continue;

		// Vérifier que le channel existe
		std::map<std::string, Channel *>::iterator it = this->_channels.find(chan);

		if (it == this->_channels.end())
		{
			client->sendReply(":ircserv 403 " + client->getNickName() + " " + chan + " :No such channel");
			continue;
		}

		Channel *channel = it->second;

		if (!channel->isInChannel(client))
		{
			client->sendReply(":ircserv 442 " + client->getNickName() + " " + chan + " :You're not on that channel");
			continue;
		}

		// motif pour partir apres les :
		std::string reason;
		if (!cmd.trailing.empty())
            reason = " :" + cmd.trailing;

		std::string partMsg = ":" + client->getNickName() + " PART " + chan + reason;

		channel->sendToAll(partMsg);
		channel->leaveChannel(client);

		if (channel->getNbrConnected() == 0)
		{
			delete channel;
			this->_channels.erase(it);
		}
	}
}

void Server::cmdTopic(Client *client, Client::parsedCommand &cmd)
{
	if (cmd.parametresMiddle.empty())
	{
		client->sendReply(formatMessage(ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS, client, ""));
		return;
	}

	std::string channel = cmd.parametresMiddle[0];
	std::map<std::string, Channel *>::iterator it = this->_channels.find(channel);
	if (it == this->_channels.end())
	{
		client->sendReply(formatMessage(ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL, client, ""));
		return;
	}
	if (!this->_channels[channel]->isInChannel(client))
	{
		client->sendReply(formatMessage(ERR_NOTONCHANNEL, MSG_NOTONCHANNEL, client, channel));
		return;
	}

	if (cmd.trailing.empty())
	{
		std::map<std::string, Channel *>::iterator it = this->_channels.find(channel);

		if (!(*it).second->haveTopic())
		{
			client->sendReply(formatMessage(ERR_NOTOPIC, MSG_NOTOPIC, client, channel));
		}
		else
		{
			client->sendReply(formatMessage(RPL_TOPIC, ":" + it->second->getTopic(), client, channel));
		}
		return;
	}

	this->_channels[channel]->setTopic(cmd.trailing, client);

	std::string topicMsg = ":" + client->getNickName() + " TOPIC " + channel + " :" + cmd.trailing;
	this->_channels[channel]->sendToAll(topicMsg);
}

void Server::cmdKick(Client *client, Client::parsedCommand &cmd)
{
	(void)client;
	(void)cmd;
}

void Server::cmdInvite(Client *client, Client::parsedCommand &cmd)
{
	if (cmd.parametresMiddle.size() < 2)
    {
        client->sendReply(":ircserv 461 " + client->getNickName()
            + " INVITE :Not enough parameters");
        return;
    }

    std::string nick = cmd.parametresMiddle[0];
    std::string channel = cmd.parametresMiddle[1];

    Client *target = NULL;

    for (std::map<int, Client *>::iterator it = this->clients.begin();
        it != this->clients.end(); ++it)
    {
        if (it->second->getNickName() == nick)
        {
            target = it->second;
            break;
        }
    }

    if (target == NULL)
    {
        client->sendReply(":ircserv 401 " + client->getNickName()
            + " " + nick + " :No such nick");
        return;
    }

	if (!target->isRegistered())
	{
    	client->sendReply(":ircserv 401 " + client->getNickName()
       	 	+ " " + nick + " :No such nick");
    	return;
	}

    std::map<std::string, Channel *>::iterator it =
        this->_channels.find(channel);

    if (it == this->_channels.end())
    {
        client->sendReply(":ircserv 403 " + client->getNickName()
            + " " + channel + " :No such channel");
        return;
    }

    Channel *chan = it->second;

    if (!chan->isInChannel(client))
    {
        client->sendReply(":ircserv 442 " + client->getNickName()
            + " " + channel + " :You're not on that channel");
        return;
    }

    if (chan->isInChannel(target))
    {
        client->sendReply(":ircserv 443 " + client->getNickName()
            + " " + nick + " " + channel
            + " :is already on channel");
        return;
    }

    target->sendReply(":" + client->getNickName()
        + " INVITE " + nick + " :" + channel);

    client->sendReply(":ircserv 341 " + client->getNickName()
        + " " + nick + " " + channel);
}

void Server::cmdMode(Client *client, Client::parsedCommand &cmd)
{

	int countNbrChann = 0;
	std::vector<std::string> modeWant;
	Channel *channelTarget = NULL;

	for (std::vector<std::string>::iterator it = cmd.parametresMiddle.begin(); it != cmd.parametresMiddle.end(); it++) // boucle pour verif si tout les args sont bons
	{
		if ((*it)[0] == '#' && countNbrChann == 0) // si le premier argument est un channel
		{
			countNbrChann++;

			std::map<std::string, Channel *>::iterator chIt = this->_channels.find((*it));

			if (chIt == this->_channels.end()) // pas de channels correspondant
			{
				client->sendReply(formatMessage(ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL, client, (*it)));
				return;
			}
			std::cerr << (*chIt).second->getName() << std::endl; // debug
			channelTarget = (*chIt).second;
		}
		else if (countNbrChann == 1) // si il existe un deuxieme args
		{
			countNbrChann++;

			for (size_t i = 0; i < (*it).size(); i++) // parse les modes voulues
			{
				if ((*it)[i] && ((*it)[i] == '+' || (*it)[i] == '-'))
				{
					if ((*it).size() > i + 1 && isModeIsHandle((*it)[i + 1]))
					{
						char del = (*it)[i];
						char mode = (*it)[i + 1];

						std::string modeAndDel;
						modeAndDel += del;
						modeAndDel += mode;
						modeWant.push_back(modeAndDel);

						i++;
						continue;
					}
					else if ((*it).size() > i + 1 && !isModeIsHandle((*it)[i + 1]))
					{
						client->sendReply(formatMessage(ERR_UNKNOWNMODE, MSG_UNKNOWNMODE, client, ""));
						return;
					}
				}
				else if (isModeIsHandle((*it)[i])) // si pas de + ou de - on suppose que c'est un +
				{
					char mode = (*it)[i];
					std::string modeAndDel = "+";

					modeAndDel += mode;
					modeWant.push_back(modeAndDel);
				}
				else
				{
					client->sendReply(formatMessage(ERR_UNKNOWNMODE, MSG_UNKNOWNMODE, client, ""));
					return;
				}
			}
		}
		else if ((*it)[0] != '#' && countNbrChann == 0) // si le premier argument est un channel sans la bonne syntaxe
		{
			client->sendReply(formatMessage(ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL, client, (*it)));
			return;
		}
	}

	if (countNbrChann == 1 && modeWant.empty()) // si aucun argument n'est donne en plus du canal on renvoi les modes actif sur le channel
	{
		client->sendReply(formatMessage(RPL_CHANNELMODEIS, channelTarget->mode.getModeActive(), client, channelTarget->getName()));
		return;
	}

	countNbrChann = 1;
	for (std::vector<std::string>::iterator it = modeWant.begin(); it != modeWant.end(); it++) // dernier check des modes si il y en a avec des attributs on check si il existe genre mdp et limites
	{
		if ((*it)[0] == '+')
		{
			std::cerr << "debug: add mode" << std::endl; // debug

			if ((*it)[1] == 'k' || (*it)[1] == 'l') // on verifie les params de k et de l (mdp et limite de nbr)
			{
				size_t isExist = countNbrChann + 1;
				if (isExist >= cmd.parametresMiddle.size())
				{
					client->sendReply(formatMessage(ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS, client, channelTarget->getName()));
					return;
				}
				if ((*it)[1] == 'l') // on verifie le param de l si c'est bien que 1 nombre positif
				{
					for (size_t i = 0; cmd.parametresMiddle[isExist].size() > i; i++)
					{
						if (!std::isdigit(cmd.parametresMiddle[isExist][i]))
						{
							client->sendReply(formatMessage(ERR_INVALIDMODEPARAM, MSG_INVALIDMODEPARAM, client, channelTarget->getName()));
							return;
						}
					}
				}
				countNbrChann++;
			}
		}
		else
		{
			std::cerr << "debug: supp mode" << std::endl; // debug

			if ((*it)[1] == 'k') // on verifie les params de k
			{
				size_t isExist = countNbrChann + 1;
				if (isExist >= cmd.parametresMiddle.size()) // pas de params on pleures
				{
					client->sendReply(formatMessage(ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS, client, channelTarget->getName()));
					return;
				}
				countNbrChann++;
			}
		}
	}

	size_t paramsNbr = countNbrChann + 1;
	if (paramsNbr != cmd.parametresMiddle.size()) // si il y a trop de parametres dans le mode, on pleures encore
	{
		client->sendReply(formatMessage(ERR_NEEDMOREPARAMS, MSG_NEEDMOREPARAMS, client, channelTarget->getName()));
		return;
	}

	// wip -------------------------------------------------
	// wip gestion des modes une fois que tout est bien valide (normalement le parsing au dessu est bon (inch'))

	// for (size_t i = 0; i < modeWant.size(); i++)
	// {
	// 	channelTarget->mode.setMode(,);
	// }
	// 	---------------------------------------------------
}

bool Server::isModeIsHandle(char &mode)
{
	std::vector<char> handleMode;

	handleMode.push_back('i');
	handleMode.push_back('t');
	handleMode.push_back('k');
	handleMode.push_back('o');
	handleMode.push_back('l');
	for (std::vector<char>::iterator it = handleMode.begin(); it != handleMode.end(); it++)
	{
		if (mode == (*it))
			return true;
	}
	return false;
}

void Server::cmdPrivmsg(Client *client, Client::parsedCommand &cmd)
{

	if (cmd.parametresMiddle.empty())
	{
		client->sendReply(formatMessage(ERR_NORECIPIENT, MSG_NORECIPIENT, client, ""));
		return;
	}

	// std::cerr << "debug :" << cmd.command << "\n nbr of parameter :" << cmd.parametresMiddle.size() << std::endl;
	// for (std::vector<std::string>::iterator it = cmd.parametresMiddle.begin(); it != cmd.parametresMiddle.end(); it++)
	// 	std::cerr << (*it).c_str() << std::endl;

	if (cmd.trailing.empty())
	{
		client->sendReply(formatMessage(ERR_NOTEXTTOSEND, MSG_NOTEXTTOSEND, client, ""));
		return;
	}

	std::string target = cmd.parametresMiddle[0];
	std::string msg = ":" + client->getNickName() + " PRIVMSG " + target + " :" + cmd.trailing;

	if (target[0] == '#')
	{
		std::map<std::string, Channel *>::iterator it = this->_channels.find(target);
		if (it == this->_channels.end()) // pas trouver le channels
		{
			client->sendReply(formatMessage(ERR_NOSUCHCHANNEL, MSG_NOSUCHCHANNEL, client, target));
			return;
		}
		if (!(*it).second->isInChannel(client))
		{
			client->sendReply(formatMessage(ERR_NOTONCHANNEL, MSG_NOTONCHANNEL, client, target));
			return;
		}
		// envoi a tous sauf expediteur
		(*it).second->sendToAllOtherClient(msg, client);
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
		client->sendReply(formatMessage(ERR_NOSUCHNICK, MSG_NOSUCHNICK, client, target));
	}
}

void Server::cmdPing(Client *client, Client::parsedCommand &cmd) // a verif je crois qu'il manques des trucs
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

void Server::broadcastQuit(Client *client, std::string reason)
{
	std::string nick = client->getNickName();

	if (reason.empty())
		reason = "Client Quit";

	std::string quitMsg = ":" + nick + " QUIT :" + reason;

	for (std::map<std::string, Channel *>::iterator it = this->_channels.begin(); it != this->_channels.end(); it++)
	{
		if ((*it).second->isInChannel(client))
		{
			(*it).second->sendToAll(quitMsg);
			(*it).second->leaveChannel(client);
		}
	}

	std::map<std::string, Channel *>::iterator it = this->_channels.begin();
	while (it != this->_channels.end())
	{
		if ((*it).second->getNbrConnected() == 0)
		{
			delete (*it).second;
			this->_channels.erase(it++);
		}
		else
			++it;
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
