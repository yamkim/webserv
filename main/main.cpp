#include <iostream>
//#include <cstring>
//#include <string>
//#include <arpa/inet.h>
// #include <sys/socket.h>
// #include <sys/errno.h>
// #include <sys/time.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <poll.h>
#include "PairArray.hpp"
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include <vector>

std::string genHttpString() {
    std::string httpStr;
    httpStr += "HTTP/1.1 200 OK\r\n";
    httpStr += "Content-Type: text/html\r\n";
    httpStr += "\r\n";
    httpStr += "<html>";
    httpStr += "<head><title>hi</title></head>";
    httpStr += "<body>";
    httpStr += "<b><center> HI! </center></b>";
    httpStr += "</body>";
    httpStr += "</html>";
    return httpStr;
}

int main(void)
{
    PairArray pollfds;
#if 1
    Socket* lSocket1 = new ListeningSocket(4200, 42);
    if (lSocket1->runSocket())
        return (1);
    lSocket1->setPollFd(POLLIN);
    pollfds.appendElement(lSocket1, PairArray::LISTENING);
#endif

#if 0
    Socket* lSocket1 = new ListeningSocket(4201, 42);
    if (lSocket1->runSocket())
        return (1);
    Socket* lSocket2 = new ListeningSocket(4202, 42);
    if (lSocket2->runSocket())
        return (1);
    Socket* lSocket3 = new ListeningSocket(4203, 42);
    if (lSocket3->runSocket())
        return (1);
    
    lSocket1->setPollFd(POLLIN);
    lSocket2->setPollFd(POLLIN);
    lSocket3->setPollFd(POLLIN);

    pollfds.appendElement(lSocket1, PairArray::LISTENING);
    pollfds.appendElement(lSocket2, PairArray::LISTENING);
    pollfds.appendElement(lSocket3, PairArray::LISTENING);
 #endif

    try {
        while (true) {
            int result = poll(pollfds.getArray(), pollfds.getSize(), 1000);
            if (result == -1) {
                throw ErrorHandler("Error: poll operation error.", ErrorHandler::CRITICAL, "Polling::run");
            } else if (result == 0) {
                std::cout << "waiting..." << std::endl;
            } else {
                pollfds.renewVector();
                pollfds.showVector();
                for (size_t i = 0; i < pollfds.getSize(); ++i) {
                    // TODO 타입을 Socket 안에 넣는 것도 고려
                    Socket* curSocket = pollfds[i].first;
                    int curSocketType = pollfds[i].second;
                    if (curSocket->getPollFd().revents & POLLIN) {
                        if (curSocketType == PairArray::LISTENING) {
                            Socket* cSocket = new ConnectionSocket(curSocket->getSocket());
                            pollfds.appendElement(cSocket, PairArray::CONNECTION);
                        } else {
                            char buffer[1024];
                            int readLength = read(pollfds[i].first->getPollFd().fd, buffer, 1024);
                            std::string httpResStr = genHttpString();
                            std::cout << "data : " << std::string(buffer, readLength) << std::endl;
                            write(pollfds[i].first->getPollFd().fd, httpResStr.data(), httpResStr.length());

                            close(pollfds[i].first->getPollFd().fd);
                            pollfds.removeElement(i--);
                            std::cout << std::endl;
                        }
                    }
                }
            }
        }
    } catch (const std::exception &error) {
        std::cout << error.what() << std::endl;
        return (1);
    }
    return (0);
}
    
