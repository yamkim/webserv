#include "Socket.hpp"

Socket::Socket(int socket) : _socket(socket)
{
}

Socket::~Socket() {
    if (_socket != -1) {
        if (close(_socket) == -1) {
        	throw ErrorHandler("Error: Can't close socket", ErrorHandler::ALERT, "Socket::~Socket");
        }
    }
}