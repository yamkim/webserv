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
        int _socket;
        NginxConfig::ServerBlock _serverConf;

    public:
        Socket(int socket_);
        Socket(int socket_, const NginxConfig::ServerBlock& serverConf);
        virtual ~Socket();
        int getSocket() const;
        NginxConfig::ServerBlock getConfig() const;
        virtual int runSocket() = 0;
};

#endif