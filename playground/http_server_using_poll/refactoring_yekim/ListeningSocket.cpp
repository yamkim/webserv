#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket() {
}

// TODO close allow function인지 확인하기..
ListeningSocket::~ListeningSocket() {
    close(_socket);
}

int ListeningSocket::getSocket() const {
    return (this->_socket);
}

void ListeningSocket::setSocket() {
    this->_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (this->_socket == -1) {
        throw "Error: server socket open error.";
    }
}

void ListeningSocket::setSocketAddress(int portNum) {
    std::memset(&(this->_socketAddr), 0, sizeof(this->_socketAddr));
    this->_socketAddr.sin_family = AF_INET;
    this->_socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    this->_socketAddr.sin_port = htons(portNum);
}

void ListeningSocket::bindSocket() {
    if ((bind(this->_socket, (struct sockaddr *) &_socketAddr, sizeof(this->_socketAddr)) == -1))
        throw "Error: server socket bind error.";
}

void ListeningSocket::listenSocket() {
    if ((listen(_socket, 42) == -1))
        throw "Error: server socket listen error.";
}

void ListeningSocket::fcntlSocket() {
    if (fcntl(_socket, F_SETFL, O_NONBLOCK) == -1)
        throw "Error: server socket fcntl error.";
}