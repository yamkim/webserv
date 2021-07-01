#ifndef SOCKET_HPP
#define SOCKET_HPP 

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include "ErrorHandler.hpp"
#include "NginxConfig.hpp"
#include "Utils.hpp"

class Socket {
    protected:
        struct sockaddr_in _socketAddr;
        int _socket;
        socklen_t _socketLen;
        NginxConfig::ServerBlock _conf;

    public:
        Socket(int socket_);
        Socket(int socket_, const NginxConfig::ServerBlock& conf_);
        virtual ~Socket();
        int getSocket() const;
        NginxConfig::ServerBlock getConfig() const;

        virtual int runSocket() = 0;
};

#endif