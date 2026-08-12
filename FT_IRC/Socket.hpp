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

class Socket
{
	public:

		Socket(int domain, std::string ip, int port);

		//create hostname in usable network information.
		int createNetwork();

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

	private:

		//Translates a hostname and a service into usable network information.
		struct addrinfo hints;
		struct addrinfo *result;

		//socket parameters
		int socketDomain;
		int socketType;
		int socketProtocol;
		int sockFd;

		//network parameter
		std::string ip;
		std::string port;

		//test functions or debugs
		void printAll();


};
