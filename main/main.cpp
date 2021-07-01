#include <iostream>
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include <vector>
#include "KernelQueue.hpp"
#include <cstdlib>

int main(int argc, char *argv[])
{
    KernelQueue kq;
    try {
        NginxConfig nginxConfig("nginx.conf");
    #if 1
    (void)argc, (void)argv;
    for (int i = 0; i < (int)nginxConfig._http.server.size(); i++) {
        // ListeningSocket* lSocket = new ListeningSocket(std::atoi(nginxConfig._http.server[i].listen.c_str()), 42);
        // ListeningSocket* lSocket = new ListeningSocket(std::atoi(nginxConfig._http.server[i].dirMap["listen"].c_str()), 42);
        ListeningSocket* lSocket = new ListeningSocket(nginxConfig._http.server[i]);
        if (lSocket->runSocket())
            return (1);
        kq.addReadEvent(lSocket->getSocket(), reinterpret_cast<void*>(lSocket));
    }
    } catch (const std::string& e) {
        std::cout << e << std::endl;
        return 1;
    }
    #else
    for (int i = 1; i < argc; i++) {
        ListeningSocket* lSocket = new ListeningSocket(std::atoi(argv[i]), 42);
        if (lSocket->runSocket())
            return (1);
        kq.addReadEvent(lSocket->getSocket(), reinterpret_cast<void*>(lSocket));
    }
    #endif
#if 1
    try {
        while (true) {
            int result = kq.getEventsIndex();
            if (result == 0) {
                std::cout << "waiting..." << std::endl;
            } else {
                for (int i = 0; i < result; ++i) {
                    Socket* instance = reinterpret_cast<Socket*>(kq.getInstance(i));
                    if (dynamic_cast<KernelQueue::PairQueue*>(instance) != NULL) {
                        // NOTE: CGI Event 발생
                        KernelQueue::PairQueue* pairQueue = reinterpret_cast<KernelQueue::PairQueue*>(instance);
                        pairQueue->stopSlave();
                    } else if (dynamic_cast<ListeningSocket*>(instance) != NULL) {
                        // NOTE: Listening Socket Event 발생
                        ConnectionSocket* cSocket = new ConnectionSocket(instance->getSocket());
                        kq.addReadEvent(cSocket->getSocket(), reinterpret_cast<void*>(cSocket));
                    } else if (dynamic_cast<ConnectionSocket*>(instance) != NULL) {
                        // NOTE: Connection Socket Event 발생
                        ConnectionSocket* cSocket = dynamic_cast<ConnectionSocket*>(instance);
                        if (kq.isClose(i)) {
                            // NOTE: 연결 종료 여부 확인
                            delete cSocket;
                        } else if (kq.isReadEvent(i)) {
                            // NOTE: Read Event
                            // ============================================================================================
                            // FIXME: HTTPRequest, HTTPResponse를 밖으로 빼고, 생성자에서 cSocket을 받는 식으로 대공사를 하면 어떨까요..?
                            // ============================================================================================
                            if (cSocket->HTTPRequestProcess(nginxConfig) == HTTPRequestHandler::FINISH) {
                                // NOTE: to Write Event
                                kq.modEventToWriteEvent(i);
                            }
                        } else if (kq.isWriteEvent(i)) {
                            // NOTE: Write Event
                            HTTPResponseHandler::Phase phase = cSocket->HTTPResponseProcess();
                            if (phase == HTTPResponseHandler::FINISH) {
                                kq.deletePairEvent(i);
                                delete cSocket;
                            } else if (phase == HTTPResponseHandler::CGI_REQ) {
                                kq.setPairEvent(i, cSocket->getCGIfd());
                            }
                        }
                    }
                }
            }
        }
    } catch (const std::exception &error) {
        std::cout << error.what() << std::endl;
        return (1);
    }
#endif
    return (0);
}
    
