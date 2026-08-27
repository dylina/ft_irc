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
#define BUFFER_SIZE 1024
#endif

class Client
{
    public:

		Client();
		Client(int fd, sockaddr_in _clientAddr);
		~Client();

		int 		getFd() const;//
		sockaddr_in getAddr();//
		std::string	getNickName() const;//
		std::string	getUserName() const;//
		std::string getData() const;//
		int 		getMode() const;//
		bool 		hasPass() const;//
		bool        isRegistered() const;

		void setNickName(std::string nick);//
		void setUserName(std::string name);
		void setMode(int mode);
		void setPass(bool val);

		void addData(std::string buffer, int clientFd);
		bool parseCommand();

		void printBufferEnhance();


	private:

		int 			_fd;
		int 			_mode;
		sockaddr_in 	_clientAddr;
		std::string 	_nickName;
        std::string 	_userName;
		std::string 	_dataBuffer;
		bool 			_hasPass;
		CommandComplete _commandsConstruct;

};
