#include "ConnectionSocket.hpp"

#include <iostream>


ConnectionSocket::ConnectionSocket(int listeningSocketFd) : Socket(-1) {
    this->_socket = accept(listeningSocketFd, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    _req = new HTTPRequestHandler(_socket);
    _res = NULL;
    this->setPollFd(POLLIN);
}

ConnectionSocket::~ConnectionSocket(){
    delete _req;
    delete _res;
}

bool ConnectionSocket::HTTPProcess(void) {
    if (_res == NULL) {
        _req->process();
        if (_req->isFinish()) {
            if (_req->isConnectionCloseByClient()) {
                std::cout << "[DEBUG] Connection CLOSED BY CLIENT" << std::endl;
                close(_socket);
                return (false);
            } else {
                std::cout << "[DEBUG] to RESPONSE" << std::endl;
                _res = new HTTPResponseHandler(_socket, _req->getURI());
                setPollFd(POLLOUT);
            }
        }
    } else {
        _res->process();
        if (_res->isFinish()) {
            std::cout << "[DEBUG] Connection CLOSE" << std::endl;
            close(_socket);
            return (false);
        }
    }
    return (true);
}

struct pollfd ConnectionSocket::getPollfd() const {
    return (this->_pollfd);
}

int ConnectionSocket::runSocket() {
    return (0);
}