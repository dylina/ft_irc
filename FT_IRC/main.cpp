#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "colors.hpp"
#include "Socket.hpp"
#include "Server.hpp"

int main(int argc, char **argv)
{
		// Domain :	AF_INET	ou	AF_INET6	ou	AF_UNIX	ou	AF_LOCAL
		//IP : localhost
		// Port: entre 1020 et 65535


	if (argc != 3)
	{
		std::cout << "Usage : ./ircserv <port> <password>" << std::endl;
		return 1;
	}


	try
	{
		std::string port = argv[1];

		// start with 127.0.0.1 == "localhost"
		Server irc(AF_INET, "127.0.0.1", port);

		irc.on();

	}
	catch (std::exception &e)
	{
		std::cerr << BOLDRED << "!Error :" << e.what() << RESET << std::endl;
		return 1;
	}


}
