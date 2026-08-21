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
- return  adress [`sockaddr_in`](../structure/sockaddr_in.md)

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

- call [listen()](documentations/functions/listen()/listen.md) and test it for now

#### variables

- [`sockaddr_in`](documentations/structure/sockaddr_in.md) _addr;
- [`Socket`]() _serverSocket;

</br>


	Server(int domain, std::string ip, std::string port)
			|			|					|
			|			|					└─> ./ircserv <PORT> <>
			|			|
			|			└> 127.0.0.1 == "localhost"
			|
			└> protocol IpvX


