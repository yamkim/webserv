#include "ConnectionSocket.hpp"

#include <iostream>


ConnectionSocket::ConnectionSocket(int listeningSocketFd) : Socket(-1) {
    this->_socket = accept(listeningSocketFd, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    this->setPollFd(POLLIN);
}

ConnectionSocket::~ConnectionSocket(){
}

struct pollfd ConnectionSocket::getPollfd() const {
    return (this->_pollfd);
}

int ConnectionSocket::runSocket() {
    return (0);
}