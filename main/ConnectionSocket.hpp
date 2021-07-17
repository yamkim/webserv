#ifndef CONNECTIONSOCKET_HPP
#define CONNECTIONSOCKET_HPP

#include <iostream>
#include "Socket.hpp"
#include "HTTPRequestHandler.hpp"
#include "HTTPResponseHandler.hpp"
#include "HTTPData.hpp"

class ConnectionSocket : public Socket {
    private:
        ConnectionSocket();
        HTTPRequestHandler* _req;
        HTTPResponseHandler* _res;
        HTTPData _data;
        NginxConfig::NginxConfig _nginxConf;
        long _dynamicBufferSize;
        bool _connectionCloseByServer;
    public:
        ConnectionSocket(int listeningSocket, const NginxConfig::ServerBlock& conf, const NginxConfig::NginxConfig& nginxConfig);
        virtual ~ConnectionSocket();
        HTTPRequestHandler::Phase HTTPRequestProcess(void);
        HTTPResponseHandler::Phase HTTPResponseProcess(void);
        int runSocket(void);
        int getCGIfd(void);
        void setConnectionData(struct sockaddr_in _serverSocketAddr, struct sockaddr_in _clientSocektAddr);
        static void ConnectionSocketKiller(void* connectionsocket);
        long getDynamicBufferSize(void);
        void setDynamicBufferSize(long dynamicBufferSize);
};

#endif