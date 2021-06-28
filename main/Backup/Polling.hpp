#ifndef POLLING_HPP
#define POLLING_HPP

#include "Array.hpp"
#include <poll.h>
#include <iostream>
#include <map>
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include "CGISession.hpp"

class Polling {
    private:
        Array<struct pollfd> pollfds;
        int _serverSocket;

    public:
        Polling(int serverSocket);
        int run(const std::string &httpResStr);
};


#endif