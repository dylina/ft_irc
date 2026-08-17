#pragma once
#include "Socket.hpp"

class Server
{
	public:

		Server(Socket socket);


	private:

		Socket _serverSocket;
		struct sockaddr _addr;
};
