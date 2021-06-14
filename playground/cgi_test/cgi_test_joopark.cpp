#include <iostream>
#include <cstring>
#include <string>
#include <poll.h>

#include "CGISession.hpp"
#include "Server.hpp"
#include "Array.hpp"

#define PYTHON_BIN "/usr/bin/python3"
// 환경에 맞게 py 파일의 절대 경로로 수정해야 합니다.
#define CGI_PATH "/Users/joohongpark/Desktop/webserv/webserv/playground/cgi_test/pycgi.py"

int main(int argc, char **argv, char **envp) {
	(void) argc;
	(void) argv;
	char buffer[1024];
	int readLength;
	CGISession *pyCGI;
	Server serv(8080, 1024);

	struct sockaddr_in clientAddress;
	socklen_t addressSize;
	int clientSocket;

	// for poll (2)
	Array<struct pollfd> fdPollList;
	struct pollfd tmp;
	int result;

	serv.makeServer();
	if (serv.isEnable() == false) {
		std::cout << "err : " << std::strerror(errno) << std::endl;
		std::cout << "at : " << "Server" << std::endl;
		return (1);
	}

	tmp.fd = serv.getSocket();
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
			for (unsigned int i = 0; i < fdPollList.size(); i++) {
				if (fdPollList[i].revents & POLLIN) {
					if (fdPollList[i].fd == serv.getSocket()) {
						clientSocket = accept(fdPollList[i].fd, (struct sockaddr *) &clientAddress, &addressSize);
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
						pyCGI = new CGISession;
						pyCGI->setCGIargs(const_cast<char*>(PYTHON_BIN), const_cast<char*>(CGI_PATH), const_cast<char*>("data=test"), envp);
						pyCGI->makeCGIProcess();
						readLength = read(fdPollList[i].fd, buffer, 1024);
						std::cout << "recv data\n" << std::string(buffer, readLength) << std::endl;
						write(fdPollList[i].fd, "HTTP/1.1 200 OK\n", 16);
						readLength = read(pyCGI->getOutputStream(), buffer, 1024);
						write(fdPollList[i].fd, buffer, readLength);
						close(fdPollList[i].fd);
						delete pyCGI;
						fdPollList.removeElement(i);
					}
					break ; // 벡터 원소 변형 후 for문 마저 돌리면 문제 발생하므로 break
				}
			}
		}
	}
	

/*
	read(pyCGI.getOutputStream(), buf, 1024);
	std::cout << buf << std::endl;
*/
	return (0);
}