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
    // NOTE: Request, Response가 나눠져 있어 둘로 나눠놨습니다.
    HTTPRequestHandler::Phase HTTPRequestProcess(void);
    // NOTE: 둘로 나눈 가장 큰 이유인데 여기서 CGI fd를 이벤트에 등록해야 해서..
    HTTPResponseHandler::Phase HTTPResponseProcess(void);
    struct pollfd getPollfd() const;
    int runSocket();
    int getCGIfd(void);
};

#endif