<!-- **This project has been created as part of the 42 curriculum by dgorceac, mleschev.** -->

<!-- Docs for all notions -->
### 	[For details of the project clic here](documentations/global_knowledge/README.md)

<!-- Docs for functions use in irc in french -->
### 	[For details of functions may be utils clic here](documentations/functions/FunctionREADME.md)

# Class of the project:

## Socket()

Call by [`Server`](#Server) constructor check all args passed and return a variables with getter;

#### getter

`Socket::getSocket()`
- return  adress;

`getDomain()`
- return  sockDomain;

`getType()`
- return  sockType;

`getProtocol()`
- return  sockProtocol;

`getFd()`
- return  sockFd;



## Server()

Create a server with socket base on args at construct and call [bind()](documentations/functions/bind()/bind.md) , or throw() an exception if have error


#### Have Members:


[on](#) :

>call listen() and test it for now

#### variables

- [`sockaddr_in`](../structure/sockaddr_in.md) _addr;
- [`Socket`]() _serverSocket;

</br>


	Server(int domain, std::string ip, std::string port)
			|			|					|
			|			|					└─> ./ircserv <PORT> <>
			|			|
			|			└> 127.0.0.1 == "localhost"
			|
			└> protocol IpvX


