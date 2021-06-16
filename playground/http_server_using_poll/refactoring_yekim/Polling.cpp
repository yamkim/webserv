#include "Polling.hpp"

int Polling::run(const std::string &httpResStr) {
    while (true) {
        int result;
        result = poll(pollfds.getArray(), pollfds.size(), 1000);
        if (result == -1) {
            throw "Error: poll operation error.";
        } else if (result == 0) {
            std::cout << "waiting..." << std::endl;
        } else {
            for (unsigned int i = 0; i < pollfds.size(); i++) {
                if (pollfds[i].revents & POLLIN) {
                    // if (pollfds[i].fd == serverSocket) {
                    if (pollfds[i].fd == this->_serverSocket) {
                        ConnectionSocket connectionSocket(this->_serverSocket);
                        struct pollfd tmp = connectionSocket.getPollfd();
                        pollfds.appendElement(tmp);
                    } else {
                        char buffer[1024];
                        int readLength;
                        readLength = read(pollfds[i].fd, buffer, 1024);
                        std::cout << "data : " << std::string(buffer, readLength) << std::endl;
                        //std::map<std::string, std::string> httpHeaders;

                        write(pollfds[i].fd, httpResStr.data(), httpResStr.length());

                        close(pollfds[i].fd);
                        pollfds.removeElement(i);
                    }
                    break ; // 벡터 원소 변형 후 for문 마저 돌리면 문제 발생하므로 break
                }
            }
        }
    }
}