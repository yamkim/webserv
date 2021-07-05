#include "ListeningSocket.hpp"

// ListeningSocket::ListeningSocket() : Socket(-1, conf) {
//     _ip = NULL;
//     _portNum = 4200;
//     _backlog = 42;
// }

// ListeningSocket::ListeningSocket(int portNum, int backlog) : Socket(-1) {
//     _ip = NULL;
//     _portNum = portNum;
//     _backlog = backlog;
// }

ListeningSocket::ListeningSocket(const NginxConfig::ServerBlock& serverConf) : Socket(-1, serverConf) {
    _portNum = atoi(Utils::getMapValue(_serverConf.dirMap, "listen").c_str());
    _backlog = 42;
}

// ListeningSocket::ListeningSocket(int portNum, int backlog, const char* ip) : Socket(-1) {
//     _ip = ip;
//     _portNum = portNum;
//     _backlog = backlog;
// }

// TODO close allow function인지 확인하기..
ListeningSocket::~ListeningSocket() {
}

void ListeningSocket::setSocket() {
    this->_socket = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)_optval, OPTVAL_BUFFER_SIZE);
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

void ListeningSocket::fcntlSocket() {
    if (fcntl(_socket, F_SETFL, O_NONBLOCK) == -1) {
        throw ErrorHandler("Error: server socket fcntl error.", ErrorHandler::CRITICAL, "ListeningSocket::fcntlSocket");
    }
}

int ListeningSocket::runSocket() {
    try {
        this->setSocket();
        this->setSocketAddress();
        this->bindSocket();
        this->listenSocket();
        this->fcntlSocket();
    } catch (const std::exception &error) {
        std::cout << error.what() << std::endl;
        return (1);
    }
    return (0);
}