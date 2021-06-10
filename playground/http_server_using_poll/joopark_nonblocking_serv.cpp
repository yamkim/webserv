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

int main(void)
{
	// common
	int serverSocket;
	int clientSocket;
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	char buffer[1024];
	std::string httpTestHeaderString;
	socklen_t addressSize;
	int readLength;

	// for poll (2)
	Array<struct pollfd> fdPollList;
	struct pollfd tmp;
	int result;

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
		std::cout << "at : " << "fcntl (2)" << std::endl;
		return (1);
	}

	// accept를 하기 위한 serverSocket도 select로 감시함.
	tmp.fd = serverSocket;
	tmp.events = POLLIN;
	fdPollList.appendElement(tmp);

	while (true) {
		result = poll(fdPollList.getArray(), fdPollList.size(), 1000);
		if (result == -1) {
			std::cout << "err : " << std::strerror(errno) << std::endl;
			std::cout << "at : " << "poll (2)" << std::endl;
			return (1);
		} else if (result == 0) {
			std::cout << "waiting..." << std::endl;
		} else {
			for (int i = 0; i < fdPollList.size(); i++) {
				if (fdPollList[i].revents & POLLIN) {
					if (fdPollList[i].fd == serverSocket) {
						clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &addressSize);
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
						readLength = read(fdPollList[i].fd, buffer, 1024);
						std::cout << "data : " << std::string(buffer, readLength) << std::endl;
						write(fdPollList[i].fd, httpTestHeaderString.data(), httpTestHeaderString.length());
						close(fdPollList[i].fd);
						fdPollList.removeElement(i);
					}
					break ; // 벡터 원소 변형 후 for문 마저 돌리면 문제 발생하므로 break
				}
			}
		}
	}
	close(serverSocket);
	return (0);
}
