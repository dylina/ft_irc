<!-- **This project has been created as part of the 42 curriculum by dgorceac, mleschev.** -->

<!-- Docs for all notions -->
### 	[For details of the project clic here](documentations/global_knowledge/README.md)

<!-- Docs for functions use in irc in french -->
### 	[For details of functions may be utils clic here](documentations/functions/FunctionREADME.md)

###	[Figma](https://www.figma.com/design/C4HZnHfigGpI6nS6wX3mII/Sans-titre?node-id=0-1&p=f&t=aRpLHCoFwRaahX5C-0)

# Class of the project:

## Socket()

Call by [`Server`](#Server) constructor check all args passed and return a variables with getter;

#### getter

`getSocket()`
- return  adress [`sockaddr_in`](documentations/structure/sockaddr_in.md)

`getDomain()`
- return  sockDomain `int`

`getType()`
- return  sockType `int`

`getProtocol()`
- return  sockProtocol `int`

`getFd()`
- return  sockFd `int`



## Server()

Create a IRC server with socket base on args at construct and call [bind()](documentations/functions/bind()/bind.md) , or throw() an exception if have error


#### Have Members:


`on()` :

- call [listen()](documentations/functions/listen()/listen.md), do a [`struct pollfd`](documentations/functions/poll()/struct_pollfd.md) and surveys on a loop;

#### variables


- [`Socket`](documentations/functions/socket()/SOCKET.md) _serverSocket;
- [`std::map<int,Client *>`](#stdmapintclient--clients) clients;
- [`std::vector<struct pollfd>`](#stdvectorstruct-pollfd-fds) fds;

</br>


	Server(int domain, std::string ip, std::string port)
			|			|					|
			|			|					└─> ./ircserv <PORT> <>
			|			|
			|			└> 127.0.0.1 == "localhost"
			|
			└> protocol IpvX

## Client

Class for attributes of client (name, mode, data buffer) stocked by pair with their fd on Server class by [`std::map<int,client *> clients`](#stdmapintclient--clients)

#### variables
- std::string _userName => contains client username

- std::string _dataBuffer => contains buffer of what server received from client fd call by `addData`

- int _mode => contains actual mode of client `wip`

#### members

- std::string	getName() const => return username `std::string`

- std::string getData() const => return inner buffer of client `wip`
- int getMode() const `wip`
- void setName(std::string name) `wip`
- void addData(char *buffer, int clientFd) `wip`
- void setMode(int mode) `wip`



# Variables and struct used

### `std::map<int,Client *> clients`


 stock une paire :
 ```
 int clientFd : Client * nouveauClient
 ```

tableau associatif permettant de retrouver toute les infos d'un client (mode, username, buffer) grace a son fd;

### `std::vector<struct pollfd> fds`

structure contenue dans [`Server`](#server) qui stock en premier le socket d'ecoute sous struct pollfd puis ensuite toute les connexion en fd sous forme dynamique



# Sources

https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/

https://www.ibm.com/docs/en/zos/2.5.0?topic=otap-clientserver-socket-programs-blocking-nonblocking-asynchronous-socket-calls

http://manpagesfr.free.fr/man/man2/listen.2.html

https://www.frameip.com/c-mode-connecte/

https://www.csd.uoc.gr/~hy556/material/tutorials/cs556-3rd-tutorial.pdf

https://www.codequoi.com/programmation-reseau-via-socket-en-c/

https://www.figma.com/board/TEviHsREY663xd6BxfD1UQ/ft_irc?node-id=1-2

https://www.techbull.com/techbull/guide/internet/irccommande.html

https://www.irchelp.org/protocol/rfc/chapter6.html

https://mathieu-lemoine.developpez.com/tutoriels/irc/protocole/?page=commandes

https://datatracker.ietf.org/doc/html/rfc1459
