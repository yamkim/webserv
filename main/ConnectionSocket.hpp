#ifndef CONNECTIONSOCKET_HPP
#define CONNECTIONSOCKET_HPP

#include "Socket.hpp"
#include "HTTPRequestHandler.hpp"
#include "HTTPResponseHandler.hpp"

class ConnectionSocket : public Socket {
private:
    ConnectionSocket();
    HTTPRequestHandler *_req;
    HTTPResponseHandler *_res;

public:
    ConnectionSocket(int listeningSocket);
    virtual ~ConnectionSocket();
    bool HTTPProcess(void);
    struct pollfd getPollfd() const;
    int runSocket();
};

#endif