#include "ConnectionSocket.hpp"

ConnectionSocket::ConnectionSocket(int serverSocket) {
    this->_socket = accept(serverSocket, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw "Error: connection socket error.";
    }
    this->_pollfd.fd = this->_socket;
    this->_pollfd.events = POLLIN;
}

struct pollfd ConnectionSocket::getPollfd() const {
    return (this->_pollfd);
}