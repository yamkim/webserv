#include "ConnectionSocket.hpp"

#include <iostream>
ConnectionSocket::ConnectionSocket(int serverSocket) : Socket(-1) {
    this->_socket = accept(serverSocket, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    std::cout << "this->socket: " << this->_socket << std::endl;
    std::cout << "client port: " << this->_socketAddr.sin_port << std::endl;
    std::cout << "client socketLen: " << this->_socketLen << std::endl;
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    this->_pollfd.fd = this->_socket;
    this->_pollfd.events = POLLIN;
}

struct pollfd ConnectionSocket::getPollfd() const {
    return (this->_pollfd);
}