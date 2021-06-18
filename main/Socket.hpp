#ifndef SOCKET_HPP
#define SOCKET_HPP 

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <poll.h>
#include "ErrorHandler.hpp"
#include <iostream>

class Socket {
    protected:
        struct pollfd _pollfd;
        struct sockaddr_in _socketAddr;
        int _socket;
        socklen_t _socketLen;

    public:
        Socket(int socket_);
        virtual ~Socket();
        void setPollFd(int events_);
        void setPollFd(struct pollfd pollfd_);
        int getSocket() const;
        struct pollfd getPollFd() const;

        virtual int runSocket() = 0;
};
std::ostream &operator<<(std::ostream &out, Socket& socket_);
#endif