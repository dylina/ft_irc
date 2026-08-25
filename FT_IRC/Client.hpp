#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <string>

class Client
{
    public:

		Client();
		Client(int fd, sockaddr_in _clientAddr);
		~Client();

		int getMode() const;
		int getFd() const;
		std::string	getNickName() const;
		std::string	getUserName() const;
		std::string getData() const;

		void setNickName(std::string name);
		void setUserkName(std::string name);
		void setData(char *buffer);
		void setMode(int mode);

    private:

		int _mode;
		int _fd;
		sockaddr_in _clientAddr;
		std::string _nickName;
        std::string _userName;
		std::string _data;
		struct pollfd _poll;

};
