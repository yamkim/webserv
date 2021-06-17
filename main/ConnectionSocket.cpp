#include "ConnectionSocket.hpp"

ConnectionSocket::ConnectionSocket(int serverSocket) {
    this->_socket = accept(serverSocket, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    this->_pollfd.fd = this->_socket;
    this->_pollfd.events = POLLIN;
}

struct pollfd ConnectionSocket::getPollfd() const {
    return (this->_pollfd);
}