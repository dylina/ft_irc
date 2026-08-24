#pragma once

#include <netinet/in.h>
#include "Client.hpp"
#include <poll.h>
#include <string>

class Client
{
    public:

		Client();

		std::string	getName() const;
		std::string getData() const;
		int getMode() const;

		void setName(std::string name);
		void setData(char *buffer);
		void setMode(int mode);

    private:

        std::string _userName;
		std::string _data;
		int _mode;

};
