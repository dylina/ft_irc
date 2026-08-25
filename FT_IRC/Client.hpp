#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <iostream>

#include "colors.hpp"
#include "Client.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

class Client
{
    public:

		Client();

		std::string	getName() const;
		std::string getData() const;
		int getMode() const;

		void setName(std::string name);
		void addData(char *buffer, int clientFd);
		void setMode(int mode);

    private:

        std::string _userName;
		std::string _dataBuffer;
		int _mode;

};
