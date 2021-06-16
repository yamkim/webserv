#ifndef CONNECTIONSOCKET_HPP
#define CONNECTIONSOCKET_HPP

#include "Socket.hpp"

class ConnectionSocket : public Socket {
private:
    ConnectionSocket();

public:
    ConnectionSocket(int serverSocket);
    struct pollfd getPollfd() const;
};

#endif