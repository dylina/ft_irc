#include "Client.hpp"


const char* Client::ErrorCreationClient::what() const throw()
{
	return "Error creation client";
}

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
	this->printBufferEnhance();
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

CommandComplete Client::parseCommand()
{
	try
	{
		CommandComplete newcommad;
		newcommad.command = "";
		newcommad.params.clear();
		newcommad._isDone = "none";


		//first parse the command
		int retCmd = 0;

		if ((retCmd = this->_dataBuffer.find("\r\n")) == -1)
			throw ErrorCreationClient();
		for (int t = 0; t < retCmd; t++)
		{
			if (newcommad.command.size() == 0 and (isalpha(_dataBuffer[t])))
				newcommad.command = _dataBuffer[t];
			else if (isspace(_dataBuffer[t]))
				break;
			else
				newcommad.command += _dataBuffer[t];
		}

		this->_dataBuffer.erase(0, retCmd);
		std::cout << _dataBuffer << std::endl;

		//first parse the parameteres in vector
		if ((retCmd = this->_dataBuffer.find("\r\n")) < 0)
			throw ErrorCreationClient();
		std::string param;
		for (int t = 0; t < ' '; t++)
		{

			if (param.size() == 0 and (isalpha(_dataBuffer[t])))
				param = _dataBuffer[t];
			else if (isspace(_dataBuffer[t]))
			{
				this->_commandsConstruct.params.push_back(param);
				_dataBuffer.erase(0, t);
				break;
			}
			else if (_dataBuffer[t] == ',')
			{
				this->_commandsConstruct.params.push_back(param);
				param.clear();
			}
			else
				param += _dataBuffer[t];
		}

		for (unsigned long commandsnbr = 0; commandsnbr < _commandsConstruct.params.size() - 1; commandsnbr++)
		{
			std::cerr << RED << "LOG " << commandsnbr << " :\n"
			<< "Command : " << newcommad.command << '\n'
			<< "Paramterers : ";
			for (std::vector<std::string>::iterator it = _commandsConstruct.params.begin() ; it != _commandsConstruct.params.end(); it++)
			std::cerr << *it << " , ";
		}
		std::cerr << RESET << std::endl;



		return newcommad;
	}
	catch(std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}

	throw ErrorCreationClient();
}
