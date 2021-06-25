#ifndef SOCKET_HPP
#define SOCKET_HPP 

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "ErrorHandler.hpp"
#include <iostream>

class Socket {
    protected:
        struct sockaddr_in _socketAddr;
        int _socket;
        socklen_t _socketLen;

    public:
        Socket(int socket_);
        virtual ~Socket();
        int getSocket() const;

        virtual int runSocket() = 0;
};

#endif