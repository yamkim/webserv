#ifndef SOCKET_HPP
#define SOCKET_HPP 

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <poll.h>
#include "ErrorHandler.hpp"

class Socket {
protected:
    struct pollfd _pollfd;
    struct sockaddr_in _socketAddr;
    int _socket;
    socklen_t _socketLen;
};
#endif