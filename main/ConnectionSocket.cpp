#include "ConnectionSocket.hpp"

#include <iostream>


ConnectionSocket::ConnectionSocket(int listeningSocketFd) : Socket(-1) {
    this->_socket = accept(listeningSocketFd, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    _proc.first = new HTTPRequestHandler(_socket);
    _proc.second = NULL;
    this->setPollFd(POLLIN);
}

ConnectionSocket::~ConnectionSocket(){
    delete _proc.first;
    delete _proc.second;
}

struct pollfd ConnectionSocket::getPollfd() const {
    return (this->_pollfd);
}

int ConnectionSocket::runSocket() {
    return (0);
}