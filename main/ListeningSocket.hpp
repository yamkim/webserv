#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include "Socket.hpp"

class ListeningSocket : public Socket {
    private:
        int _backlog;
        int _portNum;
        const char* _ip;
        ListeningSocket();

    public:
        ListeningSocket(int portNum, int backlog);
        ListeningSocket(int portNum, int backlog, const char* ip);
        virtual ~ListeningSocket();

        int getSocket() const;
        void setSocket();
        void setSocketAddress();
        void bindSocket();
        void listenSocket();
        void fcntlSocket();
        int runSocket();
};
#endif
