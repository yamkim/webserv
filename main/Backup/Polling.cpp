#include "Polling.hpp"

Polling::Polling(int serverSocket) : _serverSocket(serverSocket) {
    struct pollfd tmp;
    tmp.fd = this->_serverSocket;
    tmp.events = POLLIN;
    pollfds.appendElement(tmp); 
};

int Polling::run(const std::string &httpResStr) {
    ConnectionSocket *connectionSocket;
    std::map<int, ConnectionSocket *> connectionSockets;
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
                    if (pollfds[i].fd == this->_serverSocket) {
                        connectionSocket = new ConnectionSocket(this->_serverSocket);
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
                        //std::map<std::string, std::string> httpHeaders;

                        write(pollfds[i].fd, httpResStr.data(), httpResStr.length());

                        delete connectionSockets[pollfds[i].fd];
                        connectionSockets.erase(pollfds[i].fd);
                        std::cout << pollfds[i].fd << "is eleminated" << std::endl;
                        close(pollfds[i].fd);
                        pollfds.removeElement(i);
                        i--;
                        std::map<int, ConnectionSocket *>::iterator iter;
                        for(iter = connectionSockets.begin(); iter != connectionSockets.end() ; iter++){
                            std::cout << "[" << iter->first << ", " << iter->second << "]" << " ";
                        }
                        std::cout << std::endl;
                        #endif

                    }
                }
            }
        }
    }
}