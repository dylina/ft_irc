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

#include "colors.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512 //limitation doc irc
#endif

class Client
{
    public:

		Client();
		Client(int fd, sockaddr_in _clientAddr);
		~Client();

			// Getters
		int			getFd() const;
		sockaddr_in getAddr() const;

		std::string getName() const;//jsp si on doit garder
		std::string getNickName() const;
		std::string getUserName() const;
		std::string getData() const;

		int			getMode() const;
		bool		hasPass() const;
		bool		isRegistered() const;

		// Setters
		void setName(const std::string& name);
		void setNickName(const std::string& nick);
		void setUserName(const std::string& name);
		void setMode(int mode);
		void setPass(bool val);

			// Canale (join/part)
		void joinChannel(const std::string &name);
		void leaveChannel(const std::string &name);
		bool isInChannel(const std::string &name) const;
		std::set<std::string> getChannels() const;

			// Data / parsing
		void addData(std::string buffer, int clientFd);
		void parseCommand();

			// Envoi (bufferise ici, le send() reel se fait seulement sur POLLOUT dans Server::on())
		void queueMessage(const std::string &msg);
		bool hasPendingOutput() const;
		void flushSendBuffer();

		// Debug
		void printBufferEnhance() const;
		void printCommands();

		class ErrorCreationClient : public std::exception
		{
		public:
			const char* what() const throw();
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


		Client(sockaddr_in socketClient);
		void sendReply(const std::string& msg);
		void setFD(int fd);
    	bool hasCommands() const;
    	parsedCommand popCommand();
		Client::parsedCommand popCommand();
		std::string getData() const;
		std::string getSocket() const;
		void getAllInfo() const;

		int getMode() const;
		void addData(std::string buffer, std::string clientFd);

		class ErrorCreationClient : public std::exception
		{
			public:
				const char* what() const throw();
		};

		//Authentication and registration
		void setPassOk(bool ok);
		void setHasNick(bool ok);
		void setHasUser(bool ok);
		void setRegistered(bool ok);
		void setUsername(std::string u);
		void setRealname(std::string r);
		bool isPassOk() const;
		bool hasNick() const;
		bool hasUser() const;
		bool isRegistered() const;
		std::string getUsername() const;
		std::string getRealname() const;
		std::string getFdWithColor() const;

		bool isComplete;
		// File d'attente des commandes deja parsees, consommee par Server::executeCommand
		bool hasPendingCommands() const;

		private:
		sockaddr_in _socket;



		void setMode(int mode);
		void printBufferEnhance() const;


		std::string _dataBuffer;
		int _mode;

		std::vector<parsedCommand> _commands;
		void clearStructCommand(parsedCommand &newCommand);
		void parseCommand();
		void printCommands() const;

		int _fd;
		std::string _clientFd;
		bool _passOk;
		bool _hasNick;
		bool _hasUser;
		bool _registered;
		std::string _username;
		std::string _realname;
		std::string _lastResponse;
		std::vector<std::string> _responseSend;

		void clearStructCommand(parsedCommand& newCommand);

		// Client information
		int         _fd;
		sockaddr_in _clientAddr;

		std::string _nickName;
		std::string _userName;

		// Client state
		int         _mode;
		bool        _hasPass;

		// Received data
		std::string _dataBuffer;

		// Donnees en attente d'envoi (flush uniquement sur POLLOUT)
		std::string _sendBuffer;

		// Canale la care clientul e conectat in prezent
		std::set<std::string> _channels;

		// Parsed commands
		std::vector<parsedCommand> _commands;
		std::string _clientFd;

};
