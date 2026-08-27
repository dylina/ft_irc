#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <iostream>
#include <vector>
#include <iterator>

#include "colors.hpp"
#include "CommandComplete.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512 //limitation doc irc
#endif

class Client
{
    public:

		Client();

		std::string	getName() const;
		std::string getData() const;

		int getMode() const;
		void addData(std::string buffer, int clientFd);


		private:

		void setMode(int mode);
		void setName(std::string name);
		bool parseCommand();
		void printBufferEnhance();

        std::string _userName;
		std::string _dataBuffer;
		int _mode;
		CommandComplete _commandsConstruct;




};
