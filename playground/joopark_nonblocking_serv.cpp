#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int serverSocket;
    int clientSocket;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    char buffer[1024];
    std::string httpTestHeaderString;
    struct timeval pollingTime;
    fd_set readfdSet;
    socklen_t addressSize;
    int readLength;
    int result;
    std::vector<int> fdList;

    // HTTP Header 및 Body
    httpTestHeaderString += "HTTP/1.1 200 OK\r\n";
    httpTestHeaderString += "Content-Type: text/html\r\n";
    httpTestHeaderString += "\r\n";
    httpTestHeaderString += "<html>";
    httpTestHeaderString += "<head><title>hi</title></head>";
    httpTestHeaderString += "<body>";
    httpTestHeaderString += "<b><center> HI! </center></b>";
    httpTestHeaderString += "</body>";
    httpTestHeaderString += "</html>";

    // polling time = 1 sec
    pollingTime.tv_sec = 1;
    pollingTime.tv_usec = 0;

    // read fd set 초기화
    FD_ZERO(&readfdSet);

    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cout << "err : " << std::strerror(errno) << std::endl;
        std::cout << "at : " << "socket (2)" << std::endl;
        return (1);
    }

    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(8080);

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
        std::cout << "at : " << "listen (2)" << std::endl;
        return (1);
    }

    // non-blocking accept
    while (true) {
        // accept 시스템 콜은 fd가 논블록킹이고 큐가 비어있으면 -1을 리턴한다. (man 2 accpet)
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &addressSize);
        if (clientSocket == -1) {
            if (errno == EAGAIN) {
                // EAGAIN는 fd가 논블록킹 모드이며 큐가 비었을 때 발생하는 에러 넘버이다.
                if (fdList.empty() == true) {
                    std::cout << "queue is empty..." << std::endl;
                    usleep(1000 * 1000); // 연결 상태가 아니라면 임의로 딜레이를 부여해야 함.
                } else {
                    std::cout << "connect count : " << fdList.size() << std::endl;
                    result = select(fdList.back() + 1, &readfdSet, 0, 0, &pollingTime);
                    if (result == -1) {
                        std::cout << "err : " << std::strerror(errno) << std::endl;
                        std::cout << "at : " << "select (2)" << std::endl;
                        return (1);
                    } else if (result == 0) {
                        std::cout << "waiting..." << std::endl;
                    } else {
                        for (std::vector<int>::iterator iter = fdList.begin(); iter != fdList.end(); iter++) {
                            int testfd = *iter;
                            if (FD_ISSET(testfd, &readfdSet)) {
                                readLength = read(testfd, buffer, 1024);
                                std::cout << "data : " << std::string(buffer, readLength) << std::endl;
                                write(testfd, httpTestHeaderString.data(), httpTestHeaderString.length());
                                fdList.erase(iter);
                                FD_CLR(testfd, &readfdSet);
                                close(testfd);
                                break ; // 벡터 원소 변형 후 for문 마저 돌리면 문제 발생하므로 break
                            }
                        }
                    }
                }
            } else {
                std::cout << "err : " << std::strerror(errno) << std::endl;
                std::cout << "errno : " << (errno) << std::endl;
                std::cout << "at : " << "accept (2)" << std::endl;
                return (1);
            }
        } else {
            std::cout << "fd INSERT" << std::endl;
            FD_SET(clientSocket, &readfdSet);
            fdList.push_back(clientSocket);
            std::sort(fdList.begin(), fdList.end());
        }
    }
    close(serverSocket);
    return (0);
}
