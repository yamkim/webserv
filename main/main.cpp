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
#include "Array.hpp"
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include "Polling.hpp"
// #include <map>

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
    Array<struct pollfd> pollfds;
    //ListeningSocket server(4422, 42);
    ListeningSocket server(4200, 42);
    if (server.runSocket())
        return (1);

    struct pollfd tmp;
    tmp.fd = server.getSocket();
    tmp.events = POLLIN;
    pollfds.appendElement(tmp); 
    ConnectionSocket *connectionSocket;
    std::string httpResStr = genHttpString();
    std::map<int, ConnectionSocket *> connectionSockets;
    try {
        while (true) {
            int result;
            result = poll(pollfds.getArray(), pollfds.size(), 1000);
            if (result == -1) {
                throw ErrorHandler("Error: poll operation error.", ErrorHandler::CRITICAL, "Polling::run");
            } else if (result == 0) {
                std::cout << "waiting..." << std::endl;
            } else {
                for (unsigned int i = 0; i < pollfds.size(); i++) {
                    if (pollfds[i].revents & POLLIN) {
                        if (pollfds[i].fd == server.getSocket()) {
                            connectionSocket = new ConnectionSocket(server.getSocket());
                            struct pollfd tmp = connectionSocket->getPollfd();
                            // std::cout << "pollfds[" << i << "]: " << pollfds[i].fd << std::endl;
                            // std::cout << "tmp fd: " << tmp.fd << std::endl;
                            connectionSockets[tmp.fd] = connectionSocket;
                            pollfds.appendElement(tmp);
                        } else {
                            #if 1
                            char buffer[1024];
                            int readLength;
                            readLength = read(pollfds[i].fd, buffer, 1024);
                            std::cout << "data : " << std::string(buffer, readLength) << std::endl;

                            write(pollfds[i].fd, httpResStr.data(), httpResStr.length());

                            delete connectionSockets[pollfds[i].fd];
                            connectionSockets.erase(pollfds[i].fd);
                            std::cout << pollfds[i].fd << "is eleminated" << std::endl;
                            close(pollfds[i].fd);
                            pollfds.removeElement(i);
                            i--;
                            std::cout << std::endl;
                            #endif

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


#if 0
Polling serverPolling(server.getSocket());
try {
    serverPolling.run(genHttpString());
} catch (const std::exception &error) {
    std::cout << error.what() << std::endl;
    return (1);
}
#endif