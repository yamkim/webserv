#include "Socket.hpp"

Socket::Socket(int socket_) : _socket(socket_)
{
}

Socket::~Socket() {
    if (_socket != -1) {
        if (close(_socket) == -1) {
        	throw ErrorHandler("Error: Can't close socket", ErrorHandler::ALERT, "Socket::~Socket");
        }
    }
}

int Socket::getSocket() const {
    return (this->_socket);
}

struct pollfd Socket::getPollFd() const {
    return (this->_pollfd);
}

void Socket::setPollFd(int events_) {
    this->_pollfd.fd = _socket;
    this->_pollfd.events = events_;
}

void Socket::setPollFd(struct pollfd pollfd_) {
    this->_pollfd.fd = pollfd_.fd;
    this->_pollfd.events = pollfd_.events;
    this->_pollfd.revents = pollfd_.revents;
}

std::ostream &operator<<(std::ostream &out, Socket& socket_)
{
    struct pollfd tmp = socket_.getPollFd();
    std::cout << "Socket Values=========" << std::endl;
    std::cout << "socket: " << socket_.getSocket() << std::endl;
    std::cout << "pollfd.fd: " << tmp.fd << std::endl;
    std::cout << "pollfd.event: " << tmp.events << std::endl;
    std::cout << "pollfd.revent: " << tmp.revents << std::endl;
    return (out);
}