#include "Polling.hpp"

int Polling::run(const std::string &httpResStr) {
    while (true) {
        int result;
        result = poll(pollfds.getArray(), pollfds.size(), 1000);
        if (result == -1) {
            throw ErrorHandler("Error: poll operation error.", ErrorHandler::CRITICAL, "Polling::run");
        } else if (result == 0) {
            std::cout << "waiting..." << std::endl;
        } else {
            std::cout << "DEBUG========================" << std::endl;
            for (unsigned int i = 0; i < pollfds.size(); i++) {
                if (pollfds[i].revents & POLLIN) {
                    if (pollfds[i].fd == this->_serverSocket) {
                        ConnectionSocket connectionSocket(this->_serverSocket);
                        struct pollfd tmp = connectionSocket.getPollfd();
                        pollfds.appendElement(tmp);
                    } else {
                        #if 0
                        (void)httpResStr;
                        char buffer[1024];
                        CGISession *pyCGI;
                        pyCGI = new CGISession;
                        #define PYTHON_BIN "/usr/bin/python3"
                        #define CGI_PATH "/Users/yamkim/Documents/42Projects/webserv/main/pycgi.py" 
						try {
						    pyCGI->setCGIargs(const_cast<char*>(PYTHON_BIN), const_cast<char*>(CGI_PATH), const_cast<char*>("data=test"), NULL);
						    pyCGI->makeCGIProcess();
						} catch (const ErrorHandler& e) {
						    std::cerr << e.what() << '\n';
						    if (e.getErrorcode() == ErrorHandler::CRITICAL) {
						        // TODO : 에러 발생 시 500 server error로 핸들링
						        std::exit(1);
						    }
						}
						int readLength = read(pollfds[i].fd, buffer, 1024);
						std::cout << "recv data\n" << std::string(buffer, readLength) << std::endl;
						write(pollfds[i].fd, "HTTP/1.1 200 OK\n", 16);
						readLength = read(pyCGI->getOutputStream(), buffer, 1024);
						write(pollfds[i].fd, buffer, readLength);
						close(pollfds[i].fd);
						try {
							delete pyCGI;
						} catch(const std::exception& e) {
							std::cerr << e.what() << '\n';
						}
                        #endif
                        #if 1
                        char buffer[1024];
                        int readLength;
                        readLength = read(pollfds[i].fd, buffer, 1024);
                        std::cout << "data : " << std::string(buffer, readLength) << std::endl;
                        //std::map<std::string, std::string> httpHeaders;

                        write(pollfds[i].fd, httpResStr.data(), httpResStr.length());

                        close(pollfds[i].fd);
                        pollfds.removeElement(i);
                        #endif
                    }
                    break ; // 벡터 원소 변형 후 for문 마저 돌리면 문제 발생하므로 break
                }
            }
        }
    }
}