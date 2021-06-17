#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include "Socket.hpp"

class ListeningSocket : public Socket {
public:
    ListeningSocket();
    virtual ~ListeningSocket();

    int getSocket() const;
    void setSocket();
    void setSocketAddress(int portNum);
    void bindSocket();
    void listenSocket();
    void fcntlSocket();
};
#endif
