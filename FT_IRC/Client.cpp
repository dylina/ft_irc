#include "Client.hpp"

Client::Client() : _userName(""), _dataBuffer(""), _mode(1)
{}



std::string	Client::getName() const
{
	return this->_userName;
}

void Client::setName(std::string name)
{
	this->_userName = name;
}

void Client::addData(std::string buffer, int clientFd)
{

	this->_dataBuffer += buffer;

	std::cerr << "new data from client fd [" << clientFd << "] stored in buffer client:" << std::endl;
	this->parseCommand();
}

void Client::printBufferEnhance()
{
	std::cerr << "\"" << BOLDWHITE;
		for (size_t m = 0; m < this->_dataBuffer.size() - 1; m++)
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

void Client::setMode(int mode)
{
	this->_mode = mode;
}

std::string Client::getData() const
{
	return this->_dataBuffer;
}

int Client::getMode() const
{
	return this->_mode;
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
