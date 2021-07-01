#ifndef CONNECTIONSOCKET_HPP
#define CONNECTIONSOCKET_HPP

#include "Socket.hpp"
#include "HTTPRequestHandler.hpp"
#include "HTTPResponseHandler.hpp"
#include "HTTPData.hpp"

class ConnectionSocket : public Socket {
    private:
        ConnectionSocket();
        HTTPRequestHandler *_req;
        HTTPResponseHandler *_res;
        HTTPData _data;
    public:
        typedef struct s_reqtest {
            std::string _requestLine;
            HTTPRequestHandler::Phase _phase;
            long _contentLength;
            FileController* _fileController;
            std::map<std::string, std::string> _headers;
            std::string _headerString;
            int _connectionFd;
        } reqtest;
        typedef struct s_restest {
            HTTPResponseHandler::Phase _phase;
            std::string _root;
            FileController::Type _type;

            std::string _absolutePath;
            std::string _staticHtml;
            FileController* _file;
            CGISession* _cgi;
            NginxConfig _nginxConfig;
            std::map<std::string, std::string> _headers;
            std::string _headerString;
            int _connectionFd;
        } restest;
        ConnectionSocket(int listeningSocket);
        virtual ~ConnectionSocket();
        // NOTE: Request, Response가 나눠져 있어 둘로 나눠놨습니다.
        HTTPRequestHandler::Phase HTTPRequestProcess(void);
        // NOTE: 둘로 나눈 가장 큰 이유인데 여기서 CGI fd를 이벤트에 등록해야 해서..
        HTTPResponseHandler::Phase HTTPResponseProcess(void);
        struct pollfd getPollfd() const;
        int runSocket();
        int getCGIfd(void);
        void setConnectionData(struct sockaddr_in _serverSocketAddr, struct sockaddr_in _clientSocektAddr);
        static void ConnectionSocketKiller(void* connectionsocket);
};

#endif