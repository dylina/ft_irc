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

void Client::addData(char *buffer, int clientFd)
{

	this->_dataBuffer += buffer;

		std::cerr << "new message from client fd [" << clientFd << "] :" << std::endl;
		std::cerr << "\"" << BOLDWHITE;
		for (int m = 0; m < BUFFER_SIZE; m++)
		{
			if (_dataBuffer[m] == '\n')
				std::cerr << BOLDYELLOW << "\\n" << BOLDWHITE;
			else if (_dataBuffer[m] == '\r')
				std::cerr << BOLDYELLOW << "\\r" << BOLDWHITE;
			if (!_dataBuffer[m])
			{
				std::cerr << RESET << "\"" << std::endl;
				break;
			}
			std::cerr << _dataBuffer[m];
		}
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
