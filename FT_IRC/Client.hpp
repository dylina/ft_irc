#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <iostream>
#include <vector>
#include <iterator>
#include <sys/socket.h>

#include "colors.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512 //limitation doc irc
#endif

class Client
{
    public:

		Client(sockaddr_in socketClient);
		void sendReply(const std::string& msg);
		void setFD(int fd);
		std::string	getName() const;
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


		private:

		sockaddr_in _socket;

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
		};


		void setMode(int mode);
		void setName(std::string name);
		void printBufferEnhance() const;


        std::string _userName;
		std::string _dataBuffer;
		int _mode;

		std::vector<parsedCommand> _commands;
		void clearStructCommand(parsedCommand &newCommand);
		void parseCommand();
		void printCommands() const;

		int _fd;
		std::string _clientFd;




};
