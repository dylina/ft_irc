#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <string>

class Client
{
    public:
        
    
    private:

        int _fd;
        int _mode;
        sockaddr_in _clientAddr;
        std::string _userName;

        struct pollfd _poll;

};