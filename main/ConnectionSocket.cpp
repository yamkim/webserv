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

HTTPRequestHandler::Phase ConnectionSocket::HTTPRequestProcess(void) {
    HTTPRequestHandler::Phase phase;
    phase = _req->process();
    if (phase == HTTPRequestHandler::FINISH) {
        _res = new HTTPResponseHandler(_socket, _req->getURI());
    }
    return (phase);
}

HTTPResponseHandler::Phase ConnectionSocket::HTTPResponseProcess(void) {
    HTTPResponseHandler::Phase phase;
    phase = _res->process();
    return (phase);
}

struct pollfd ConnectionSocket::getPollfd() const {
    return (this->_pollfd);
}

int ConnectionSocket::runSocket() {
    return (0);
}

int ConnectionSocket::getCGIfd(void) {
    return (_res->getCGIfd());
}