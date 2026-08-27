#include "Client.hpp"

Client::Client() : _userName(""), _dataBuffer(""), _mode(1)
{}

std::string	Client::getUserName() const
{
	return this->_userName;
}

std::string	Client::getNickName() const
{
	return this->_nickName;
}

int Client::getFd() const
{
	return this->_fd;
}

sockaddr_in Client::getAddr()
{
	return _clientAddr;
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

void Client::setNickName(std::string name)
{
	this->_nickName = name;
}

void Client::setUserName(std::string name)
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

void Client::addData(std::string buffer, int clientFd)
{

	this->_dataBuffer += buffer;

	std::cerr << "new data from client fd [" << this->_fd << "] stored in buffer client:" << std::endl;
	this->parseCommand();
	(void)clientFd;
}

void Client::printBufferEnhance()
{
	std::cerr << "\"" << BOLDWHITE;
		for (size_t m = 0; m < this->_dataBuffer.size() - 1; m++)//-1 underflow?
		{
			if (_dataBuffer[m] == '\n')
				std::cerr << BOLDYELLOW << "\\n" << BOLDWHITE;
			else if (_dataBuffer[m] == '\r')
				std::cerr << BOLDYELLOW << "\\r" << BOLDWHITE;
			else if (!_dataBuffer[m])
			{
				std::cerr << RESET << "\"" << std::endl;
				break;
			}
			else
				std::cerr << _dataBuffer[m];
		}
		std::cerr << RESET << "\"" << std::endl;
}

bool Client::parseCommand()
{
	if (!this->_commandsConstruct._isComplete)
	{
		this->printBufferEnhance(); //throw an error en vrai
		return false;
	}
	// faut parser la commande et la stocker si elle est bonne

	return true;
}
