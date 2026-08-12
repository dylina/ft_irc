#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "Socket.hpp"

int main()
{

		// Domain :	AF_INET		AF_INET6	AF_UNIX		AF_LOCAL
		//IP : localhost
		// Port: entre 0 et 65535

	try{
		Socket(AF_INET, "localhost", 100);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error :" << e.what() << '\n';
	}

}
