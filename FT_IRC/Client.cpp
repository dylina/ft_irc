#include "Client.hpp"
#include <iostream>
#include <sys/socket.h>

const char* Client::ErrorCreationClient::what() const throw()
{
	return "Error creation client";
}

Client::Client()
	: _fd(-1),
	  _clientAddr(),
	  _nickName(""),
	  _userName(""),
	  _mode(1),
	  _hasPass(false),
	  _dataBuffer("")
{
}

Client::Client(int fd, sockaddr_in clientAddr)
	: _fd(fd),
	  _clientAddr(clientAddr),
	  _nickName(""),
	  _userName(""),
	  _mode(1),
	  _hasPass(false),
	  _dataBuffer("")
{
}

Client::~Client()
{
}

int Client::getFd() const
{
	return this->_fd;
}

sockaddr_in Client::getAddr() const
{
	return _clientAddr;
}

std::string Client::getName() const//a voir si on garde
{
	return this->_nickName;
}

std::string	Client::getNickName() const
{
	return this->_nickName;
}

std::string	Client::getUserName() const
{
	return this->_userName;
}

std::string	Client::getData() const
{
	return this->_dataBuffer;
}

int Client::getMode() const
{
	return this->_mode;
}

bool Client::hasPass() const
{
	return this->_hasPass;
}

bool Client::isRegistered() const
{
	return this->_hasPass && !this->_nickName.empty() && !this->_userName.empty();;
}

void Client::setName(const std::string& name)
{
	this->_nickName = name;
}
void Client::setNickName(const std::string& nick)
{
	this->_nickName = nick;
}

void Client::setUserName(const std::string& name)
{
	this->_userName = name;
}

void Client::setMode(int mode)
{
	this->_mode = mode;
}

void Client::setPass(bool val)
{
	this->_hasPass = val;
}

bool Client::hasPendingCommands() const
{
	return !this->_commands.empty();
}

Client::parsedCommand Client::popCommand()
{
	Client::parsedCommand front = this->_commands.front();
	this->_commands.erase(this->_commands.begin());
	return front;
}

void Client::queueMessage(const std::string &msg)
{
	this->_sendBuffer += msg;
}

bool Client::hasPendingOutput() const
{
	return !this->_sendBuffer.empty();
}

void Client::flushSendBuffer()
{
	if (this->_sendBuffer.empty())
		return;

	//appele uniquement depuis Server::on() en reponse a un POLLOUT : jamais d'appel "libre" a send()
	ssize_t sent = send(this->_fd, this->_sendBuffer.c_str(), this->_sendBuffer.size(), 0);

	//pas d'inspection d'errno : en cas d'echec on retente simplement au prochain cycle POLLOUT
	if (sent > 0)
		this->_sendBuffer.erase(0, static_cast<size_t>(sent));
}

void Client::joinChannel(const std::string &name)
{
	this->_channels.insert(name);
}

void Client::leaveChannel(const std::string &name)
{
	this->_channels.erase(name);
}

bool Client::isInChannel(const std::string &name) const
{
	return this->_channels.find(name) != this->_channels.end();
}

std::set<std::string> Client::getChannels() const
{
	return this->_channels;
}

void Client::clearStructCommand(Client::parsedCommand &newCommand)
{
		newCommand.isComplete = false;
		newCommand.parametresMiddle.clear();
		newCommand.prefix.clear();
		newCommand.trailing.clear();
		newCommand.command.clear();
}

void Client::parseCommand()
{
	size_t end = _dataBuffer.find("\r\n");

	if (end != std::string::npos)
	{
		struct parsedCommand newCommand;
		clearStructCommand(newCommand);

		//parse prefix (optionnel, rarement envoye par un client)
		if (!this->_dataBuffer.empty() && this->_dataBuffer[0] == ':')
		{
			size_t posSpace = this->_dataBuffer.find_first_of(" ");
			if (posSpace != std::string::npos)
			{
				newCommand.prefix = _dataBuffer.substr(0, posSpace);
				this->_dataBuffer.erase(0, posSpace + 1);
			}
		}

		//parse command
		if (newCommand.command.empty())
		{
			size_t posSpace = _dataBuffer.find_first_of(" ");
			size_t posEnd = _dataBuffer.find_first_of("\r\n");
			if (posSpace != std::string::npos && (posEnd == std::string::npos || posSpace < posEnd))
			{
				newCommand.command = _dataBuffer.substr(0, posSpace);
				_dataBuffer.erase(0, posSpace + 1);
			}
			else if (posEnd != std::string::npos)
			{
				//commande sans aucun parametre (ex: "QUIT\r\n")
				newCommand.command = _dataBuffer.substr(0, posEnd);
				_dataBuffer.erase(0, posEnd + 2);
			}
		}

		//parse params : uniquement separes par des espaces au niveau protocole.
		//les listes "a,b,c" (JOIN/PART/PRIVMSG/KICK) restent un seul parametre brut,
		//c'est au handler de la commande de les re-splitter sur la virgule.
		while (!this->_dataBuffer.empty() && this->_dataBuffer[0] != ':')
		{
			size_t posSpace = this->_dataBuffer.find_first_of(" ");
			size_t posEnd = this->_dataBuffer.find_first_of("\r\n");

			if (posSpace != std::string::npos && (posEnd == std::string::npos || posSpace < posEnd))
			{
				std::string newParam = _dataBuffer.substr(0, posSpace);
				if (!newParam.empty())
					newCommand.parametresMiddle.push_back(newParam);
				this->_dataBuffer.erase(0, posSpace + 1);
			}
			else if (posEnd != std::string::npos)
			{
				std::string newParam = _dataBuffer.substr(0, posEnd);
				if (!newParam.empty())
					newCommand.parametresMiddle.push_back(newParam);
				this->_dataBuffer.erase(0, posEnd + 2);
				break;
			}
			else
				break; //securite : ne devrait pas arriver puisque "\r\n" a deja ete trouve plus haut
		}

		//parse trailing (":ceci est le dernier parametre, avec espaces")
		if (!this->_dataBuffer.empty() && this->_dataBuffer[0] == ':')
		{
			size_t posEnd = this->_dataBuffer.find_first_of("\r\n");
			if (posEnd != std::string::npos)
			{
				newCommand.trailing = _dataBuffer.substr(1, posEnd - 1);
				this->_dataBuffer.erase(0, posEnd + 2);
			}
		}

		newCommand.isComplete = true;
		this->_commands.push_back(newCommand);

	}
	else
		std::cerr << "commande partiel" << std::endl;
}

void Client::addData(std::string buffer, int clientFd)
{
	
	std::cerr << "new data from client fd [" << clientFd << "] stored in buffer client:" << std::endl;
	this->_dataBuffer += buffer;
	
	this->printBufferEnhance();
	
	while (this->_dataBuffer.find("\r\n") != std::string::npos)
	this->parseCommand();
	
	std::cout << "commands received from client " << clientFd << std::endl;
	this->printCommands();
	
	
}

void Client::printCommands()
{

}

void Client::printBufferEnhance() const
{
	if (this->_dataBuffer.empty())
	{
		std::cout << "Client "<< this->_clientFd <<" have no data in attempt" << std::endl;
		return;
	}
	std::cerr <<  BRIGHT_CYAN"\"" << RESET  << BOLD BRIGHT_WHITE;
		for (size_t m = 0; m < this->_dataBuffer.size(); m++)
		{
			if (_dataBuffer[m] == '\n')
				std::cerr << BOLD BRIGHT_YELLOW << "\\n" << BOLD BRIGHT_WHITE;
			else if (_dataBuffer[m] == '\r')
				std::cerr << BOLD BRIGHT_YELLOW << "\\r" << BOLD BRIGHT_WHITE;
			else if (!_dataBuffer[m])
			{
				std::cerr << BRIGHT_CYAN << "\"" << RESET << std::endl;
				break;
			}
			else
				std::cerr << _dataBuffer[m];
		}
		std::cerr << BRIGHT_CYAN << "\"" << RESET << std::endl;
}
