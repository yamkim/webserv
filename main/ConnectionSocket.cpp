#include "ConnectionSocket.hpp"

#include <iostream>


ConnectionSocket::ConnectionSocket(int listeningSocketFd) : Socket(-1) {
    this->_socket = accept(listeningSocketFd, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    _req = new HTTPRequestHandler(_socket);
    _res = NULL;
}

ConnectionSocket::~ConnectionSocket(){
    delete _req;
    delete _res;
}

HTTPRequestHandler::Phase ConnectionSocket::HTTPRequestProcess(void) {
    HTTPRequestHandler::Phase phase;
    try {
        phase = _req->process(_connectionData);
        if (phase == HTTPRequestHandler::FINISH) {
            _res = new HTTPResponseHandler(_socket, _req->getURI());
        }
    } catch (const std::exception &error) {
        std::cout << error.what() << std::endl;
        _connectionData.StatusCode = 400; // Bad Request
        // TODO: response에서 StatusCode를 인식해서 동작하게 해야 함.
        phase = HTTPRequestHandler::FINISH;
    }
    return (phase);
}

HTTPResponseHandler::Phase ConnectionSocket::HTTPResponseProcess(void) {
    HTTPResponseHandler::Phase phase;
    phase = _res->process(_connectionData);
    return (phase);
}

int ConnectionSocket::runSocket() {
    return (0);
}

int ConnectionSocket::getCGIfd(void) {
    return (_res->getCGIfd());
}