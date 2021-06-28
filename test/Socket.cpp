#include "Socket.hpp"

Socket::Socket(int socket_) : _socket(socket_)
{
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&_optval, (int)sizeof(_optval));
}

Socket::~Socket() {
    if (_socket != -1) {
        if (close(_socket) == -1) {
        	throw ErrorHandler("Error: Can't close socket", ErrorHandler::ALERT, "Socket::~Socket");
        }
    }
}

int Socket::getSocket() const {
    return (this->_socket);
}