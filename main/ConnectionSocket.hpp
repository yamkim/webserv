#ifndef CONNECTIONSOCKET_HPP
#define CONNECTIONSOCKET_HPP

#include "Socket.hpp"
#include "HTTPRequestHandler.hpp"
#include "HTTPResponseHandler.hpp"

class ConnectionSocket : public Socket {
private:
    ConnectionSocket();

public:
    std::pair<HTTPRequestHandler *, HTTPResponseHandler *> _proc;
    ConnectionSocket(int listeningSocket);
    virtual ~ConnectionSocket();

    struct pollfd getPollfd() const;
    int runSocket();
};

#endif