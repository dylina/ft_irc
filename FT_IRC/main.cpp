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
#include <cstdlib>
#include <ctime>
#include <csignal>

volatile sig_atomic_t g_running = 1;

void handleSignal(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
		g_running = 0;
}

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

	struct sigaction sa;
	sa.sa_handler = handleSignal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);


	try
	{
		std::string port = argv[1];
    	std::string password = argv[2];  // ← récupérer le password
		// start with 127.0.0.1 == "localhost"

    	Server irc(AF_INET, "127.0.0.1", port, password);  // ← passer au serveur
		std::srand(std::time(NULL));

		irc.on();

	}
	catch (std::exception &e)
	{
		std::cerr << BRIGHT_RED << "!Error :" << e.what() << RESET << std::endl;
		return 1;
	}


}
