#include "Socket.hpp"

Socket::Socket(int socket_) : _socket(socket_)
{
}

Socket::Socket(int socket_, const NginxConfig::ServerBlock& conf) : _socket(socket_), _conf(conf)
{
}

Socket::~Socket() {
    if (_socket != -1) {
        if (close(_socket) == -1) {
        	throw ErrorHandler("Error: Can't close socket", ErrorHandler::ALERT, "Socket::~Socket");
        }
    }
}

int Socket::getSocket() const {
    return _socket;
}

NginxConfig::ServerBlock Socket::getConfig() const {
    return _conf;
}