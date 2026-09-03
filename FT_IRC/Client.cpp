#include "Client.hpp"
#include <iostream>

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

		//parse prefix
		if (newCommand.prefix.empty() && *this->_dataBuffer.begin() == ':')
		{
			std::string::iterator it = _dataBuffer.begin();
			std::string prefix = _dataBuffer.substr(0, (this->_dataBuffer.find(" ")));
			this->_dataBuffer.erase(it, it + _dataBuffer.find_first_of(" ") + 1);
			newCommand.prefix = prefix;
		}

		//parse command
		if (newCommand.command.empty())
		{
			size_t pos = _dataBuffer.find(' ');
			if (pos != std::string::npos)
			{
				newCommand.command = _dataBuffer.substr(0, pos);
				_dataBuffer.erase(0, pos + 1);
			}
		}

		//parse params without trailing
		if (newCommand.parametresMiddle.empty())
		{
			size_t posSpaceStart = this->_dataBuffer.find_first_of(" ");
			size_t posCommaStart = this->_dataBuffer.find_first_of(",");
			while (posCommaStart != std::string::npos || posSpaceStart != std::string::npos)
			{
				std::string::iterator nit = _dataBuffer.begin();

				if ((*nit) == ':')
					break;

				size_t posComma = this->_dataBuffer.find_first_of(",");
				size_t posSpace = this->_dataBuffer.find_first_of(" ");
				size_t posEnd = this->_dataBuffer.find_first_of("\r\n");

				if (posComma != std::string::npos && posComma < posSpace && posComma < posEnd)
				{
					std::string newParam = _dataBuffer.substr(0, posComma);
					newCommand.parametresMiddle.push_back(newParam);
					this->_dataBuffer.erase(0, posComma);
				}
				else if (posSpace != std::string::npos && posSpace < posEnd && posSpace < posComma)
				{
					std::string newParam = _dataBuffer.substr(0, posSpace);
					newCommand.parametresMiddle.push_back(newParam);
					this->_dataBuffer.erase(0, posSpace + 1);
				}
				else if (posEnd != std::string::npos)
				{
					std::string newParam = _dataBuffer.substr(0, posEnd);
					newCommand.parametresMiddle.push_back(newParam);
					this->_dataBuffer.erase(0, posEnd + 2);
					break;
				}
			}
		}


		if (*this->_dataBuffer.begin() == ':')
		{
			size_t posEnd = this->_dataBuffer.find_first_of("\r\n");
			if (posEnd != std::string::npos)
			{
				newCommand.trailing = _dataBuffer.substr(0, posEnd);
				this->_dataBuffer.erase(0, posEnd + 2);
			}
		}

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


