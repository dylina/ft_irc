#include "Client.hpp"

Client::Client() : _userName(""), _data(""), _mode(1)
{}



std::string	Client::getName() const
{
	return this->_userName;
}

void Client::setName(std::string name)
{
	this->_userName = name;
}

void Client::setData(char *buffer)
{
	this->_data = buffer;
}

void Client::setMode(int mode)
{
	this->_mode = mode;
}

std::string Client::getData() const
{
	return this->_data;
}

int Client::getMode() const
{
	return this->_mode;
}
