#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "Socket.hpp"
#include "Server.hpp"

int main(int argc, char **argv)
{
		// Domain :	AF_INET	ou	AF_INET6	ou	AF_UNIX	ou	AF_LOCAL
		//IP : localhost
		// Port: entre 0 et 65535


	if (argc != 3)
	{
		std::cout << "Usage : ./ircserv <port> <password>" << std::endl;
		return 1;
	}


	try
	{
		char *port = argv[1];
		Socket socket(AF_INET, "localhost", port);
	}
	catch (...)
	{
		return 1;
	}


}
