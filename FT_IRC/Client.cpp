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

void Client::printCommands()
{
	for (size_t cmds = 0; cmds < this->_commands.size(); cmds++)
	{
		std::cerr << "Command number " << cmds << std::endl;

		std::cerr			<< "Prefix : ";

		if (this->_commands[cmds].prefix.empty())
			std::cerr << BOLDRED << "NO PREFIX\n" << RESET;
		else
			std::cerr << BOLDMAGENTA << this->_commands[cmds].prefix << '\n' << RESET;

		std::cerr			<< "Command :" << BOLDMAGENTA << _commands[cmds].command << '\n' << RESET
							<< "Params :";

		if (_commands[cmds].parametresMiddle.empty())
			std::cerr << BOLDRED << "NO PARAMS\n" << RESET;
		else
		{
			for (size_t i = 0; i < _commands[cmds].parametresMiddle.size(); i++)
				std::cerr << '\"' << BOLDMAGENTA << _commands[cmds].parametresMiddle[i] << RESET << '\"';
			std::cerr << std::endl;
		}

		std::cerr			<< "Trailing :";

		if (_commands[cmds].trailing.empty())
			std::cerr << BOLDRED << "NO TRAILING\n" << RESET;
		else
			std::cerr << BOLDMAGENTA << _commands[cmds].trailing << '\n' << RESET << std::endl;
		std::cerr << std::endl;
	}
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

void Client::printBufferEnhance()
{
	std::cerr << "\"" << BOLDWHITE;
		for (size_t m = 0; m < this->_dataBuffer.size(); m++)
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


