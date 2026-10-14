#include "Client.hpp"
#include <iostream>
#include <sys/socket.h>
#include <cctype>

const char *Client::ErrorCreationClient::what() const throw()
{
	return "Error creation client";
}

Client::Client(sockaddr_in socketClient)
	: _socket(socketClient),
	  _dataBuffer(""),
	  _mode(-1),
	  _fd(-1),
	  _passOk(false),
	  _hasNick(false),
	  _hasUser(false),
	  _registered(false),
	  _username(""),
	  _nickname(""),
	  _lastResponse(""),
	  _responseSend()
{
}

std::string Client::getFdWithColor() const
{
	return this->_clientFd;
}

static std::string addRandomColorsForClient(int Clientfd)
{
	std::stringstream fd;
	fd << Clientfd;

	int colors = std::rand() % 256;

	std::stringstream rdmcolor;
	rdmcolor << "\033[38;5;"
			 << colors
			 << "m"
			 << Clientfd
			 << RESET;

	return rdmcolor.str();
}

void Client::setFD(int fd)
{
	this->_fd = fd;
	this->_clientFd = addRandomColorsForClient(fd);
}

void Client::sendReply(const std::string &msg)
{
	std::string reply = msg + "\r\n";
	std::cerr << BOLD BRIGHT_CYAN << "SENDING: " << msg << RESET << std::endl; // ← ajouter
	this->_lastResponse = BOLD BRIGHT_CYAN + msg + RESET;
	this->_responseSend.push_back(_lastResponse);
	this->queueMessage(reply);
}

void Client::getSocket(std::ostream& out) const
{
	out << "Address : " << this->_socket.sin_addr.s_addr << '\n'
		<< "Family : " << this->_socket.sin_family << '\n'
		<< "Port : " << this->_socket.sin_port << '\n'
		<< "Sin Zero : " << this->_socket.sin_zero << std::endl;
}

void Client::getAllInfo(std::ostream& out) const
{
	out << "------------new--------------" << std::endl;

	if (this->isRegistered())
		out << BOLD_GREEN << "This user is Registered" << RESET << std::endl;
	else
		out << BOLD_RED << "! This user is Not Registered !" << RESET << std::endl;

	this->getSocket(out);

	out << "Mode : " << this->getMode() << '\n';

	out << "User Name : ";
	if (this->_username.empty())
		out << BG_BRIGHT_RED BOLD BRIGHT_WHITE << "Username EMPTY" << RESET << '\n';
	else
		out << this->getUsername() << '\n';

	out << "Nick Name : ";
	if (this->_nickname.empty())
		out << BG_BRIGHT_RED BOLD BRIGHT_WHITE << "Nickname EMPTY" << RESET << '\n';
	else
		out << this->getNickname() << '\n';

	this->printBufferEnhance(out);
	this->printCommands(out);

	if (!this->_lastResponse.empty())
	{
		out << "responses sends :" << std::endl;

		for (std::vector<std::string>::const_iterator it = _responseSend.begin();
			 it != this->_responseSend.end(); ++it)
			out << *it << std::endl;
	}
	else
		out << "No response from server send" << std::endl;

	out << "------------end--------------" << std::endl;
}

int Client::getFd() const
{
	return this->_fd;
}

sockaddr_in Client::getAddr() const
{
	return _clientAddr;
}

bool Client::hasPass() const
{
	return this->_hasPass;
}

bool Client::hasPendingCommands() const
{
	return !this->_commands.empty();
}

std::string Client::getNickName() const
{
	return this->_nickname;
}

void Client::setNickName(const std::string &nick)
{
	this->_nickname = nick;
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

	// appele uniquement depuis Server::on() en reponse a un POLLOUT : jamais d'appel "libre" a send()
	ssize_t sent = send(this->_fd, this->_sendBuffer.c_str(), this->_sendBuffer.size(), 0);

	// pas d'inspection d'errno : en cas d'echec on retente simplement au prochain cycle POLLOUT
	if (sent > 0)
		this->_sendBuffer.erase(0, static_cast<size_t>(sent));
}


// je deplace sa dans Channel.cpp
// les clients ne geres pas les channels -----------

// void Client::joinChannel(const std::string &name)
// {
// 	this->_channels.insert(name);
// }

// void Client::leaveChannel(const std::string &name)
// {
// 	this->_channels.erase(name);
// }

// bool Client::isInChannel(const std::string &name) const
// {
// 	return this->_channels.find(name) != this->_channels.end();
// }---------------------------------------------

std::vector<std::string> Client::getChannels() const
{
	return this->_channels;
}

void Client::clearStructCommand(Client::parsedCommand &newCommand)
{
	// newCommand.isComplete = false;
	newCommand.parametresMiddle.clear();
	newCommand.prefix.clear();
	newCommand.trailing.clear();
	newCommand.command.clear();
}

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
	for (size_t i = 0; i < cmd.command.size(); ++i)
		cmd.command[i] = std::toupper(static_cast<unsigned char>(cmd.command[i]));

	if (!cmd.command.empty())
		this->_commands.push_back(cmd);
}




void Client::printCommands(std::ostream& out) const
{
	for (size_t cmds = 0; cmds < this->_commands.size(); cmds++)
	{
		out << "\nCommand in waiting list number "
			<< cmds + 1 << "/" << this->_commands.size()
			<< " for fd [" << _clientFd << "]" << std::endl;

		out << "Prefix : ";

		if (this->_commands[cmds].prefix.empty())
			out << BG_BRIGHT_RED BRIGHT_WHITE
				<< "NO PREFIX" << RESET << '\n';
		else
			out << BOLD BRIGHT_MAGENTA
				<< this->_commands[cmds].prefix << RESET << '\n';

		out << "Command :"
			<< BOLD BRIGHT_MAGENTA
			<< _commands[cmds].command << RESET << '\n';

		out << "Params :";

		if (_commands[cmds].parametresMiddle.empty())
		{
			out << BOLD BG_BRIGHT_RED
				<< "NO PARAMS\n" << RESET;
		}
		else
		{
			for (size_t i = 0;
				 i < _commands[cmds].parametresMiddle.size();
				 i++)
			{
				out << " ";
				out << '\"'
					<< BOLD BRIGHT_MAGENTA
					<< _commands[cmds].parametresMiddle[i]
					<< RESET
					<< '\"';
			}
			out << std::endl;
		}

		out << "Trailing :";

		if (_commands[cmds].trailing.empty())
			out << BG_BRIGHT_RED BOLD BRIGHT_WHITE
				<< "NO TRAILING" << RESET << '\n';
		else
			out << BOLD BRIGHT_MAGENTA
				<< _commands[cmds].trailing << RESET << std::endl;

		out << std::endl;
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
}

void Client::printBufferEnhance() const
{
	if (this->_dataBuffer.empty())
	{
		std::cerr << "Client " << this->_clientFd
			<< " have no data in attempt" << std::endl;
		return;
	}

	std::cerr << BRIGHT_CYAN "\"" << RESET << BOLD BRIGHT_WHITE;

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

void Client::printBufferEnhance(std::ostream& out) const
{
	if (this->_dataBuffer.empty())
	{
		out << "Client " << this->_clientFd
			<< " have no data in attempt" << std::endl;
		return;
	}

	out << BRIGHT_CYAN "\"" << RESET << BOLD BRIGHT_WHITE;

	for (size_t m = 0; m < this->_dataBuffer.size(); m++)
	{
		if (_dataBuffer[m] == '\n')
			out << BOLD BRIGHT_YELLOW << "\\n" << BOLD BRIGHT_WHITE;
		else if (_dataBuffer[m] == '\r')
			out << BOLD BRIGHT_YELLOW << "\\r" << BOLD BRIGHT_WHITE;
		else if (!_dataBuffer[m])
		{
			out << BRIGHT_CYAN << "\"" << RESET << std::endl;
			break;
		}
		else
			out << _dataBuffer[m];
	}

	out << BRIGHT_CYAN << "\"" << RESET << std::endl;
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

std::string Client::getNickname() const
{
	return this->_nickname;
}

void Client::leave(std::string &name)
{
	for (std::vector<std::string>::iterator it = this->_channels.begin(); it != _channels.end(); it++)
	{
		if (name == (*it))
			this->_channels.erase(it);
	}
}
void Client::join(std::string &name)
{
	this->_channels.push_back(name);
}
