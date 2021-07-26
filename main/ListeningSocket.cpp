#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(const NginxConfig::ServerBlock& serverConf, int backlog) : Socket(-1, serverConf) {
    _portNum = std::atoi(Utils::getMapValue(_serverConf.dirMap, "listen").c_str());
    _backlog = backlog;
}

ListeningSocket::~ListeningSocket() {
}

void ListeningSocket::setSocket() {
    char optval[1024];
    this->_socket = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)optval, 1024);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: server socket open error.", ErrorHandler::CRITICAL, "ListeningSocket::setSocket");
    }
}

void ListeningSocket::setSocketAddress() {
    std::memset(&(this->_socketAddr), 0, sizeof(this->_socketAddr));
    this->_socketAddr.sin_family = AF_INET;
    this->_socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->_socketAddr.sin_port = htons(this->_portNum);
}

void ListeningSocket::bindSocket() {
    if ((bind(this->_socket, (struct sockaddr *) &_socketAddr, sizeof(this->_socketAddr)) == -1)) {
        throw ErrorHandler("Error: server socket bind error.", ErrorHandler::CRITICAL, "ListeningSocket::bindSocket");
    }
}

void ListeningSocket::listenSocket() {
    if ((listen(_socket, _backlog) == -1)) {
        throw ErrorHandler("Error: server socket listen error.", ErrorHandler::CRITICAL, "ListeningSocket::listenSocket");
    }
}

int ListeningSocket::runSocket() {
    this->setSocket();
    this->setSocketAddress();
    this->bindSocket();
    this->listenSocket();
    return (0);
}