#include "ConnectionSocket.hpp"

#include <iostream>


ConnectionSocket::ConnectionSocket(int listeningSocketFd) : Socket(-1) {
    std::cout << "listeningSocketFd: " << listeningSocketFd << std::endl;
    this->_socket = accept(listeningSocketFd, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    std::cout << "ConnectionSocket(): " << this->_socket << std::endl;
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    #if 0
    _req = new HTTPRequestHandler(_socket);
    _res = NULL;
    #endif
}

ConnectionSocket::~ConnectionSocket(){
    #if 0
    delete _req;
    delete _res;
    #endif
}

#if 0
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
#endif

int ConnectionSocket::runSocket() {
    return (0);
}

#if 0
int ConnectionSocket::getCGIfd(void) {
    return (_res->getCGIfd());
}
#endif