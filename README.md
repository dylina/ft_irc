<!-- **This project has been created as part of the 42 curriculum by dgorceac, mleschev.** -->

<!-- Docs for all notions -->
### 	[For details of the project clic here](documentations/global_knowledge/README.md)

<!-- Docs for functions use in irc in french -->
### 	[For details of functions may be utils clic here](documentations/functions/FunctionREADME.md)

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

- [`sockaddr_in`](documentations/structure/sockaddr_in.md) _addr;
- [`Socket`](documentations/functions/socket()/SOCKET.md) _serverSocket;

</br>


	Server(int domain, std::string ip, std::string port)
			|			|					|
			|			|					└─> ./ircserv <PORT> <>
			|			|
			|			└> 127.0.0.1 == "localhost"
			|
			└> protocol IpvX



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
