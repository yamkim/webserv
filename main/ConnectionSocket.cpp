#include "ConnectionSocket.hpp"

#include <iostream>


ConnectionSocket::ConnectionSocket(int listeningSocketFd, const NginxConfig::ServerBlock& conf, const NginxConfig& nginxConf) : Socket(-1, conf), _nginxConf(nginxConf) {
    struct sockaddr_in myAddr;
    this->_socket = accept(listeningSocketFd, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    if (getsockname(this->_socket, (struct sockaddr *) &myAddr, &this->_socketLen) == -1) {
        throw ErrorHandler("Error: getsockname error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    setConnectionData(_socketAddr, myAddr);
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
        phase = _req->process(_data);
        if (phase == HTTPRequestHandler::FINISH) {
            _res = new HTTPResponseHandler(_socket);
        }
    } catch (const std::exception &error) {
        std::cout << error.what() << std::endl;
        _data._statusCode = 400; // Bad Request
        // TODO: response에서 StatusCode를 인식해서 동작하게 해야 함.
        phase = HTTPRequestHandler::FINISH;
    }
    return (phase);
}

void ConnectionSocket::setConnectionData(struct sockaddr_in _serverSocketAddr, struct sockaddr_in _clientSocketAddr) {
    _data._clientIP = std::string(inet_ntoa(_serverSocketAddr.sin_addr));
    _data._clientPort = ntohs(_serverSocketAddr.sin_port);
    _data._hostIP = std::string(inet_ntoa(_clientSocketAddr.sin_addr));
    _data._hostPort = ntohs(_clientSocketAddr.sin_port);
}

HTTPResponseHandler::Phase ConnectionSocket::HTTPResponseProcess(void) {
    HTTPResponseHandler::Phase phase;
    phase = _res->process(_data);
    return (phase);
}

int ConnectionSocket::runSocket() {
    return (0);
}

int ConnectionSocket::getCGIfd(void) {
    return (_res->getCGIfd());
}

void ConnectionSocket::ConnectionSocketKiller(void* connectionsocket) {
    delete reinterpret_cast<ConnectionSocket*>(connectionsocket);
}
