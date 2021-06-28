#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include "Socket.hpp"

#define OPTVAL_BUFFER_SIZE 1024
class ListeningSocket : public Socket {
    private:
        int _backlog;
        int _portNum;
        const char* _ip;
        char _optval[OPTVAL_BUFFER_SIZE];
        ListeningSocket();

    public:
        ListeningSocket(int portNum, int backlog);
        ListeningSocket(int portNum, int backlog, const char* ip);
        virtual ~ListeningSocket();

        void setSocket();
        void setSocketAddress();
        void bindSocket();
        void listenSocket();
        void fcntlSocket();
        int runSocket();
};
#endif
