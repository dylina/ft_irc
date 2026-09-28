#include "Client.hpp"


const char* Client::ErrorCreationClient::what() const throw()
{
	return "Error creation client";
}

Client::Client(sockaddr_in socketClient)
    : _socket(socketClient),
      _userName(""),
      _dataBuffer(""),
      _mode(1),
      _fd(-1),
      _passOk(false),
      _hasNick(false),
      _hasUser(false),
      _registered(false),
      _username(""),
      _realname("")
{}

void Client::setFD(int fd)
{
	this->_fd = fd;
}

void Client::sendReply(const std::string& msg)
{
    std::string reply = msg + "\r\n";
    std::cerr << BOLD BRIGHT_CYAN << "SENDING: " << msg << RESET << std::endl; // ← ajouter
    send(this->_fd, reply.c_str(), reply.length(), 0);
}

std::string	Client::getName() const
{
	return this->_userName;
}

void Client::setName(std::string name)
{
	this->_userName = name;
}

std::string Client::getSocket() const
{
	std::string ret;
	std::cout	<< "Address : " << this->_socket.sin_addr.s_addr << '\n'
				<< "Family : " << this->_socket.sin_family << '\n'
				<< "Port :" << this->_socket.sin_port << '\n'
				<< "Sin Zero : " << this->_socket.sin_zero << std::endl;
	return ret;
}

void Client::getAllInfo() const
{

	this->getSocket();
	std::cout << "Mode : " << this->getMode() << '\n';
	std::cout << "Name : ";
	if (this->_userName.empty())
		std::cout << BG_BRIGHT_RED BOLD BRIGHT_WHITE << "NAME EMPTY" << RESET <<'\n';
	else
		std::cout << this->getName() << '\n';
	std::cout << "Buffer : ";
	if (_dataBuffer.empty())
		std::cout << BG_BRIGHT_RED BOLD BRIGHT_WHITE << "BUFFER EMPTY" << RESET <<'\n';
	else
		printBufferEnhance();

	this->printCommands();
	printBufferEnhance();
}

void Client::clearStructCommand(Client::parsedCommand &newCommand)
{
		// newCommand.isComplete = false;
		newCommand.parametresMiddle.clear();
		newCommand.prefix.clear();
		newCommand.trailing.clear();
		newCommand.command.clear();
}

// void Client::parseCommand()
// {
// 	size_t end = _dataBuffer.find("\r\n");

// 	if (end != std::string::npos)
// 	{
// 		struct parsedCommand newCommand;
// 		clearStructCommand(newCommand);

// 		//parse prefix
// 		if (newCommand.prefix.empty() && *this->_dataBuffer.begin() == ':')
// 		{
// 			std::string::iterator it = _dataBuffer.begin();
// 			std::string prefix = _dataBuffer.substr(0, (this->_dataBuffer.find(" ")));
// 			this->_dataBuffer.erase(it, it + _dataBuffer.find_first_of(" ") + 1);
// 			newCommand.prefix = prefix;
// 		}

// 		//
// 		if (newCommand.command.empty())
// 		{
// 			size_t pos = _dataBuffer.find(' ');
// 			if (pos != std::string::npos)
// 			{
// 				newCommand.command = _dataBuffer.substr(0, pos);
// 				_dataBuffer.erase(0, pos + 1);
// 			}
// 		}

// 		//parse params without trailing
// 		if (newCommand.parametresMiddle.empty())
// 		{
// 			size_t posSpaceStart = this->_dataBuffer.find_first_of(" ");
// 			size_t posCommaStart = this->_dataBuffer.find_first_of(",");
// 			while (posCommaStart != std::string::npos || posSpaceStart != std::string::npos)
// 			{
// 				std::string::iterator nit = _dataBuffer.begin();

// 				if ((*nit) == ':')
// 					break;

// 				size_t posComma = this->_dataBuffer.find_first_of(",");
// 				size_t posSpace = this->_dataBuffer.find_first_of(" ");
// 				size_t posEnd = this->_dataBuffer.find_first_of("\r\n");

// 				if (posComma != std::string::npos && posComma < posSpace && posComma < posEnd)
// 				{
// 					std::string newParam = _dataBuffer.substr(0, posComma);
// 					newCommand.parametresMiddle.push_back(newParam);
// 					this->_dataBuffer.erase(0, posComma);
// 				}
// 				else if (posSpace != std::string::npos && posSpace < posEnd && posSpace < posComma)
// 				{
// 					std::string newParam = _dataBuffer.substr(0, posSpace);
// 					newCommand.parametresMiddle.push_back(newParam);
// 					this->_dataBuffer.erase(0, posSpace + 1);
// 				}
// 				else if (posEnd != std::string::npos)
// 				{
// 					std::string newParam = _dataBuffer.substr(0, posEnd);
// 					newCommand.parametresMiddle.push_back(newParam);
// 					this->_dataBuffer.erase(0, posEnd + 2);
// 					break;
// 				}
// 			}
// 		}


// 		if (*this->_dataBuffer.begin() == ':')
// 		{
// 			size_t posEnd = this->_dataBuffer.find_first_of("\r\n");
// 			if (posEnd != std::string::npos)
// 			{
// 				newCommand.trailing = _dataBuffer.substr(0, posEnd);
// 				this->_dataBuffer.erase(0, posEnd + 2);
// 			}
// 		}

// 		this->_commands.push_back(newCommand);

// 	}
// 	else
// 		std::cerr << "commande partiel" << std::endl;
// }

void Client::parseCommand()
{
	size_t end = this->_dataBuffer.find("\r\n");
	if (end == std::string::npos)
		return;

	std::string line = this->_dataBuffer.substr(0, end);
	this->_dataBuffer.erase(0, end + 2);

	if (line.empty())
		return;

	parsedCommand cmd;
	size_t pos = 0;
	size_t sp;

	if (line[0] == ':')
	{
		sp = line.find(' ');
		if (sp == std::string::npos)
			return;
		cmd.prefix = line.substr(1, sp - 1);
		pos = sp;
	}
	while (pos < line.size() && line[pos] == ' ')
		pos++;
	sp = line.find(' ', pos);
	if (sp == std::string::npos)
	{
		cmd.command = line.substr(pos);
		pos = line.size();
	}
	else
	{
		cmd.command = line.substr(pos, sp - pos);
		pos = sp;
	}
	while (pos < line.size())
	{
		while (pos < line.size() && line[pos] == ' ')
			pos++;
		if (pos >= line.size())
			break;
		if (line[pos] == ':')
		{
			cmd.trailing = line.substr(pos + 1);
			break;
		}
		sp = line.find(' ', pos);
		if (sp == std::string::npos)
		{
			cmd.parametresMiddle.push_back(line.substr(pos));
			break;
		}
		cmd.parametresMiddle.push_back(line.substr(pos, sp - pos));
		pos = sp;
	}

	if (!cmd.command.empty())
		this->_commands.push_back(cmd);
}


void Client::printCommands() const
{
	for (size_t cmds = 0; cmds < this->_commands.size(); cmds++)
	{
		std::cerr << "\nCommand in waiting list number " << cmds + 1 << "/" << this->_commands.size() << " for fd ["<< _clientFd <<"]" << std::endl;

		std::cerr			<< "Prefix : ";

		if (this->_commands[cmds].prefix.empty())
			std::cerr << BG_BRIGHT_RED BRIGHT_WHITE << "NO PREFIX" << RESET << '\n';
		else
			std::cerr << BOLD BRIGHT_MAGENTA << this->_commands[cmds].prefix << RESET <<'\n';

		std::cerr			<< "Command :" << BOLD BRIGHT_MAGENTA << _commands[cmds].command << RESET <<'\n'
							<< "Params :";

		if (_commands[cmds].parametresMiddle.empty())
			std::cerr << BOLD BG_BRIGHT_RED << "NO PARAMS\n" << RESET;
		else
		{
			for (size_t i = 0; i < _commands[cmds].parametresMiddle.size(); i++)
			{
				if (i < _commands[cmds].parametresMiddle.size())
					std::cout << " ";
				std::cerr << '\"' << BOLD BRIGHT_MAGENTA << _commands[cmds].parametresMiddle[i] << RESET << '\"';
			}
			std::cerr << std::endl;
		}

		std::cerr			<< "Trailing :";

		if (_commands[cmds].trailing.empty())
			std::cerr << BG_BRIGHT_RED BOLD BRIGHT_WHITE << "NO TRAILING" << RESET << '\n';
		else
			std::cerr << BOLD BRIGHT_MAGENTA << _commands[cmds].trailing << RESET << std::endl;
		std::cerr << std::endl;
	}
}


void Client::addData(std::string buffer, std::string clientFd)
{
	this->_clientFd = clientFd;
	std::cerr << "new data from client fd [" << clientFd << "] stored in buffer client:" << std::endl;
	this->_dataBuffer += buffer;

	this->printBufferEnhance();

	while (this->_dataBuffer.find("\r\n") != std::string::npos)
		this->parseCommand();

	this->getAllInfo();


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

bool Client::hasCommands() const
{
    return !this->_commands.empty();
}

Client::parsedCommand Client::popCommand()
{
    parsedCommand cmd = this->_commands.front();
    this->_commands.erase(this->_commands.begin());
    return cmd;
}

void Client::setPassOk(bool ok)
{
    this->_passOk = ok;
}

void Client::setHasNick(bool ok)
{
    this->_hasNick = ok;
}

void Client::setHasUser(bool ok)
{
    this->_hasUser = ok;
}

void Client::setRegistered(bool ok)
{
    this->_registered = ok;
}

void Client::setUsername(std::string u)
{
    this->_username = u;
}

void Client::setRealname(std::string r)
{
    this->_realname = r;
}

bool Client::isPassOk() const
{
    return this->_passOk;
}

bool Client::hasNick() const
{
    return this->_hasNick;
}

bool Client::hasUser() const
{
    return this->_hasUser;
}

bool Client::isRegistered() const
{
    return this->_registered;
}

std::string Client::getUsername() const
{
    return this->_username;
}