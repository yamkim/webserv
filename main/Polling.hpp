#ifndef POLLING_HPP
#define POLLING_HPP

#include "Array.hpp"
#include <poll.h>
#include <iostream>
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include "CGISession.hpp"

class Polling {
private:
    Array<struct pollfd> pollfds;
    int _serverSocket;

public:
    Polling(int serverSocket) : _serverSocket(serverSocket) {
	struct pollfd tmp;
	tmp.fd = this->_serverSocket;
	tmp.events = POLLIN;
	pollfds.appendElement(tmp); 
    };
    int run(const std::string &httpResStr);
};


#endif