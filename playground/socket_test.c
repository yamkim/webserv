#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 50
char buffer[BUFFER_SIZE] = "Hello, World\n";

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Input port number as argv[1]\n");
        return -1;
    }

    int serverSocket;
    int clientSocket;

    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize;

    // TCP 연결지향형, ipv4 도메인을 위한 소켓 생성
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    // 주소 초기화 후 IP주소 및 포트 지정
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; // ipv4 타입
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // ip 주소
    serverAddress.sin_port = htons(atoi(argv[1])); // port 세팅

    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("Can not Bind\n");
        return -1;
    }

    if (listen(serverSocket, 5) == -1) {
        printf("Listen Fail\n");
        return -1;
    }

    while (1) {
        clientAddressSize = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressSize);
        if (clientSocket == -1) {
            printf("Listen Fail\n");
            return -1;
        }
        write(clientSocket, buffer, BUFFER_SIZE);
        close(clientSocket);
    }
    close(serverSocket);
}