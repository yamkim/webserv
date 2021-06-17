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
    ListeningSocket server(4422, 42);
    try {
        server.setSocket();
        server.setSocketAddress();
        server.bindSocket();
        server.listenSocket();
        server.fcntlSocket();
    } catch (const std::exception &error) {
        std::cout << error.what() << std::endl;
        return (1);
    }

    Polling serverPolling(server.getSocket());
    try {
        serverPolling.run(genHttpString());
    } catch (const std::exception &error) {
        std::cout << error.what() << std::endl;
        return (1);
    }
    return (0);
}

