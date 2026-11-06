#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <iostream>
#include <vector>
#include <iterator>
#include <sys/socket.h>
#include <sstream>
#include <cstdlib>
#include <set>
#include <ostream>
#include <algorithm>

#include "colors.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512 // limitation doc irc
#endif


class Client
{
public:

	struct parsedCommand;

	Client(sockaddr_in socketClient);

	// Getters

		// Authentication and registration

		bool isRegistered() const;
		std::string getNickName() const;
		bool hasPass() const;
		bool isPassOk() const;
		bool hasNick() const;
		bool hasUser() const;
		std::string getUsername() const;
		std::string getNickname() const;
		std::string getPrefix() const;

	int getFd() const;
	sockaddr_in getAddr() const;
	std::string getData() const;
	int getMode() const;
	std::string getFdWithColor() const;
	void getSocket(std::ostream& out) const;
	void getAllInfo(std::ostream& out) const;
	bool isComplete;
	bool hasCommands() const;

		// File d'attente des commandes deja parsees, consommee par Server::executeCommand
		bool hasPendingCommands() const;

	//  setters

	void setNickName(const std::string &nick);
	void setFD(int fd);
	void setPassOk(bool ok);
	void setHasNick(bool ok);
	void setHasUser(bool ok);
	void setRegistered(bool ok);
	void setUsername(std::string u);
	void setMode(int mode);
	void addData(std::string buffer, std::string clientFd);


		// Canale (join/part) ---> deplacer dans channel.hpp pour eviter que ce soit les clients qui controle les channels
		// void joinChannel(const std::string &name);
		// void leaveChannel(const std::string &name);
		// bool isInChannel(const std::string &name) const;
		std::vector<std::string> &getChannels();
		void leave(std::string &name);
		void join(std::string &name);

	// Data / parsing

	void parseCommand();

	// Envoi (bufferise ici, le send() reel se fait seulement sur POLLOUT dans Server::on())

	void queueMessage(const std::string &msg);
	bool hasPendingOutput() const;
	void flushSendBuffer();
	void sendReply(const std::string &msg);
	parsedCommand popCommand();

	// Debug

	void printBufferEnhance() const;
	void printBufferEnhance(std::ostream& out) const;
	void printCommands(std::ostream& out) const;

	class ErrorCreationClient : public std::exception
	{
		public:
			const char *what() const throw();
	};

	struct parsedCommand
	{
		/*
		<message>
			│
			├── ':' <prefix> <SPACE>
			│     │
			│     └── toto!user@localhost
			│
			├── <command>
			│     └── PRIVMSG
			│											==> : toto!user@localhost  PRIVMSG  #general  :Salut tout le monde  \r\n
			├── <params>
			│     │
			│     ├── <SPACE> <middle>
			│     │             └── #general
			│     │
			│     └── <SPACE> ':' <trailing>
			│                         └── Salut tout le monde
			│
			└── <crlf>
				└── \r\n
		*/

		std::string prefix;
		std::string command;
		std::vector<std::string> parametresMiddle;
		std::string trailing;
		bool isComplete;
	};

private:
	sockaddr_in _socket;

	std::string _dataBuffer;
	int _mode;

	std::vector<parsedCommand> _commands;
	void clearStructCommand(parsedCommand &newCommand);

	int _fd;
	std::string _clientFd;
	bool _passOk;
	bool _hasNick;
	bool _hasUser;
	bool _registered;
	std::string _username; // username
	std::string _nickname; // nickname
	std::string _lastResponse;
	std::vector<std::string> _responseSend;

	sockaddr_in _clientAddr;

	// Client state
	bool _hasPass;

	// Donnees en attente d'envoi (flush uniquement sur POLLOUT)
	std::string _sendBuffer;

	// Canale la care clientul e conectat in prezent
	std::vector<std::string> _channels;
};
