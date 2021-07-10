#include "ConnectionSocket.hpp"

ConnectionSocket::ConnectionSocket(int listeningSocketFd, const NginxConfig::ServerBlock& serverConf, const NginxConfig& nginxConf) : Socket(-1, serverConf), _nginxConf(nginxConf) {
    struct sockaddr_in myAddr;
    this->_socket = accept(listeningSocketFd, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    if (getsockname(this->_socket, (struct sockaddr *) &myAddr, &this->_socketLen) == -1) {
        throw ErrorHandler("Error: getsockname error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    setConnectionData(_socketAddr, myAddr);
    _req = new HTTPRequestHandler(_socket, _serverConf, _nginxConf);
    _res = NULL;
    _dynamicBufferSize = 0;
}

ConnectionSocket::~ConnectionSocket(){
    delete _req;
    delete _res;
}

HTTPRequestHandler::Phase ConnectionSocket::HTTPRequestProcess(void) {
    HTTPRequestHandler::Phase phase;
    try {
        phase = _req->process(_data);
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        phase = HTTPRequestHandler::FINISH;
    }
    if (phase == HTTPRequestHandler::FINISH) {
        _res = new HTTPResponseHandler(_socket, _serverConf, _nginxConf);
    }
    return (phase);
}

void ConnectionSocket::setConnectionData(struct sockaddr_in _serverSocketAddr, struct sockaddr_in _clientSocketAddr) {
    std::stringstream portString;
    _data._clientIP = std::string(inet_ntoa(_serverSocketAddr.sin_addr));
    portString << ntohs(_serverSocketAddr.sin_port);
    _data._clientPort = std::string(portString.str());
    _data._hostIP = std::string(inet_ntoa(_clientSocketAddr.sin_addr));
    portString.clear();
    portString << ntohs(_clientSocketAddr.sin_port);
    _data._hostPort = std::string(portString.str());
}

HTTPResponseHandler::Phase ConnectionSocket::HTTPResponseProcess(void) {
    HTTPResponseHandler::Phase phase;
    phase = _res->process(_data, getDynamicBufferSize());
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

long ConnectionSocket::getDynamicBufferSize(void) {
    return (_dynamicBufferSize);
}
void ConnectionSocket::setDynamicBufferSize(long dynamicBufferSize) {
    _dynamicBufferSize = dynamicBufferSize;
}
