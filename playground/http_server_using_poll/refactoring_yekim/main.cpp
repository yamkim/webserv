#include <iostream>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include "Array.hpp"
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include "Polling.hpp"
#include <map>

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
    ListeningSocket server;
    try {
        server.setSocket();
        server.setSocketAddress();
        server.bindSocket();
        server.listenSocket();
        server.fcntlSocket();
    } catch (const char *error) {
        std::cout << error << std::endl;
    }

    Polling serverPolling(server.getSocket());
    try {
        serverPolling.run(genHttpString());
    } catch (const char *error) {
        std::cout << error << std::endl;
    }


    #if 0
    // accept를 하기 위한 serverSocket도 select로 감시함.
    Array<struct pollfd> fdPollList;
    struct pollfd tmp;
    //tmp.fd = serverSocket;
    tmp.fd = server.getSocket();
    tmp.events = POLLIN;
    fdPollList.appendElement(tmp);

    while (true) {
        int result;
        result = poll(fdPollList.getArray(), fdPollList.size(), 1000);
        if (result == -1) {
            std::cout << "err : " << std::strerror(errno) << std::endl;
            std::cout << "at : " << "poll (2)" << std::endl;
            return (1);
        } else if (result == 0) {
            std::cout << "waiting..." << std::endl;
        } else {
            for (unsigned int i = 0; i < fdPollList.size(); i++) {
                if (fdPollList[i].revents & POLLIN) {
                    // if (fdPollList[i].fd == serverSocket) {
                    if (fdPollList[i].fd == server.getSocket()) {
                        int clientSocket;
                        struct sockaddr_in clientAddress;
                        socklen_t addressSize;
                        // clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &addressSize);
                        clientSocket = accept(server.getSocket(), (struct sockaddr *) &clientAddress, &addressSize);
                        if (clientSocket == -1) {
                            std::cout << "err : " << std::strerror(errno) << std::endl;
                            std::cout << "at : " << "accept (2)" << std::endl;
                            return (1);
                        }
                        std::cout << "fd INSERT" << std::endl;
                        tmp.fd = clientSocket;
                        tmp.events = POLLIN;
                        fdPollList.appendElement(tmp);
                    } else {
                        char buffer[1024];
                        int readLength;
                        readLength = read(fdPollList[i].fd, buffer, 1024);
                        std::cout << "data : " << std::string(buffer, readLength) << std::endl;
                        std::map<std::string, std::string> httpHeaders;

                        write(fdPollList[i].fd, httpTestHeaderString.data(), httpTestHeaderString.length());


                        close(fdPollList[i].fd);
                        fdPollList.removeElement(i);
                    }
                    break ; // 벡터 원소 변형 후 for문 마저 돌리면 문제 발생하므로 break
                }
            }
        }
    }
//    close(serverSocket);
    #endif
    return (0);
}

#if 0
int serverSocket;
serverSocket = socket(PF_INET, SOCK_STREAM, 0);
if (serverSocket == -1) {
    std::cout << "err : " << std::strerror(errno) << std::endl;
    std::cout << "at : " << "socket (2)" << std::endl;
    return (1);
}

struct sockaddr_in serverAddress;
std::memset(&serverAddress, 0, sizeof(serverAddress));
serverAddress.sin_family = AF_INET;
serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
serverAddress.sin_port = htons(4242);

if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
    std::cout << "err : " << std::strerror(errno) << std::endl;
    std::cout << "at : " << "bind (2)" << std::endl;
    return (1);
}
if (listen(serverSocket, 42) == -1) {
    std::cout << "err : " << std::strerror(errno) << std::endl;
    std::cout << "at : " << "listen (2)" << std::endl;
    return (1);
}

// fcntl 시스템 콜을 이용해 소켓 파일 디스크립터가 논블록킹으로 동작되게 함.
if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1) {
    std::cout << "err : " << std::strerror(errno) << std::endl;
    std::cout << "at : " << "fcntl (2)" << std::endl;
    return (1);
}
#endif