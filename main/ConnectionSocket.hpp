#ifndef CONNECTIONSOCKET_HPP
#define CONNECTIONSOCKET_HPP

#include "Socket.hpp"

class ConnectionSocket : public Socket {
private:
    ConnectionSocket();

public:
    ConnectionSocket(int listeningSocket);
    virtual ~ConnectionSocket();

    struct pollfd getPollfd() const;
    int runSocket();
};

#endif