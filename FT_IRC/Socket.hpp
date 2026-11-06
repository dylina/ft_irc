#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <unistd.h>


class Socket
{
	public:

		Socket(int domain, std::string ip, std::string port);
		~Socket();

		class ErrorOfSocket : public std::exception
		{
				public:
					const char* what() const throw();
		};
		class NoIp : public std::exception
		{
				public:
					const char* what() const throw();
		};
		class NoPort : public std::exception
		{
				public:
					const char* what() const throw();
		};

		struct sockaddr_in &getSocket();
		int getDomain() const;
		int getType() const;
		int getProtocol() const;
		int getFd() const;

		//test functions or debugs
		void printAll();

	private:

		struct sockaddr_in adress;


		//socket parameters :

		int sockDomain;
		int sockType;
		int sockProtocol;
		int sockFd;

		//Network parameter ip
		std::string ip;

		//Network parameter port
		std::string port;

};
