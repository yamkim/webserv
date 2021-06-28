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
        HTTPRequestHandler::Phase phase;
        phase = _req->process();
        if (phase == HTTPRequestHandler::CONNECTION_CLOSE) {
            std::cout << "[DEBUG] Connection CLOSED BY CLIENT" << std::endl;
            close(_socket);
            return (false);
        } else if (phase == HTTPRequestHandler::FINISH) {
            std::cout << "[DEBUG] to RESPONSE" << std::endl;
            _res = new HTTPResponseHandler(_socket, _req->getURI());
            setPollFd(POLLOUT);
        }
    } else {
        HTTPResponseHandler::Phase phase;
        phase = _res->process();
        if (phase == HTTPResponseHandler::FINISH) {
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