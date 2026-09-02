#include "Server.hpp"

const char* Server::BindErr::what() const throw()
{
	return "bind() error";
}

const char* Server::ListenErr::what() const throw()
{
	return "listen() error";
}

const char* Server::AcceptErr::what() const throw()
{
	return "accept() error";
}

const char* Server::RecvError::what() const throw()
{
	return "recv() error";
}


Server::Server(int domain, std::string ip, std::string port) : _serverSocket(domain, ip, port)
{

	if (	bind(
			this->_serverSocket.getFd(),
			reinterpret_cast<const struct sockaddr*>(&this->_serverSocket.getSocket()),
			sizeof(this->_serverSocket.getSocket())
			) == -1 )
	{
		perror("bind");
		throw BindErr();
	}
	std::cerr << BOLD BRIGHT_GREEN << "Server bind() success!" << RESET << std::endl;

}

void Server::on()
{
	if (listen(this->_serverSocket.getFd(), 128) == -1) //~ pour avoir plusieur client en mm temps
	{
		perror("listen");
		throw ListenErr();
	}
	std::cout << "Server listening on " << ntohs(this->_serverSocket.getSocket().sin_port) << " .." << std::endl;

	struct pollfd server;
	server.fd = this->_serverSocket.getFd();
	server.events = POLLIN;
	server.revents = 0;

	this->fds.push_back(server);

	// Loop principale de poll qui check les fds en continue
	while (true)
	{
		int retPoll = poll(&this->fds[0], this->fds.size(), -1);
		if (retPoll == -1)
		{
			std::cerr << BOLD BRIGHT_RED << "Error in poll() loop" << RESET << std::endl;
			break;
		}

		for (size_t i = 0; i < this->fds.size(); ++i)
		{
			// connexion flags : POLLIN = Il y a des données en attente de lecture.
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == this->_serverSocket.getFd())
				{
					sockaddr_in clientSocket;
					socklen_t clientLen = sizeof(clientSocket);

					std::cerr << "log: connexion of new client ..." << std::endl;
					int clientFd = accept(fds[i].fd, reinterpret_cast<struct sockaddr *>(&clientSocket), &clientLen);

					try
					{
						this->addNewClient(clientFd, clientSocket);
					}
					catch(const std::exception& e)
					{
						std::cerr << BOLD BRIGHT_RED << e.what() << RESET << std::endl;
					}
				}
				else
				{
					try
					{
						this->dataReceived(fds, i);
					}
					catch(const std::exception& e)
					{
						std::cerr << BOLD BRIGHT_RED << e.what() << RESET << std::endl;
					}

				}
			}
		}
	}

}

static std::string addRandomColorsForClient(int Clientfd)
{
	std::stringstream fd;
	fd << Clientfd;

	int colors = std::rand() % 256;

	std::stringstream rdmcolor;
	rdmcolor	<< "\033[38;5;"
				<< colors
				<< "m"
				<< Clientfd
				<< RESET;



	return rdmcolor.str();
}


void Server::addNewClient(int clientFd, sockaddr_in sockAddrClient)
{
	if (clientFd < 0)
		throw AcceptErr();
	struct pollfd newClientStruct;

	newClientStruct.fd = clientFd;
	newClientStruct.events = POLLIN;
	newClientStruct.revents = 0;

	this->fds.push_back(newClientStruct);

	Client *newClientClass = new Client(sockAddrClient);
	newClientClass->setFD(clientFd);
	this->clients[newClientStruct.fd] = newClientClass;



	std::vector<struct pollfd>::iterator it = fds.end() - 1;
	std::cerr << GREEN << "New client add, fd: [" << (*it).fd << "], connected" << RESET << std::endl;
}

void Server::dataReceived(std::vector<struct pollfd> &fds, int i)
{
	char buffer[BUFFER_SIZE];

	int retRecv = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0); //~risk d'overflow
	buffer[retRecv] = 0;
	//client data received
	if (retRecv > 0)
	{
		buffer[retRecv] = 0;
		std::string data(buffer, retRecv);
		std::cerr << retRecv << " bytes received from " << fds[i].fd << " fd" << std::endl;



		this->clients[fds[i].fd]->addData(data, addRandomColorsForClient(fds[i].fd));
	}
	//client lost
	else if (retRecv == 0)
	{
		std::vector<struct pollfd>::iterator it = fds.begin() + i;
		std::cerr << "client fd [" << (*it).fd << "] disconnected" << std::endl;

    	delete this->clients[(*it).fd]; //memory leaked
		this->clients.erase((*it).fd);
		close((*it).fd);
		this->fds.erase(it);
	}
	// error
	else
		std::cerr << "Error client received in fd [" << fds[i].fd << "]" << std::endl;

}

