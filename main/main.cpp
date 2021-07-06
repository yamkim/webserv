#include <iostream>
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include <vector>
#include "KernelQueue.hpp"
#include <cstdlib>
#include "Timer.hpp"

int main(int argc, char *argv[])
{
    KernelQueue kq;
    NginxConfig nginxConfig("nginx.conf");
    try {
    #if 1
    (void)argc, (void)argv;
    for (int i = 0; i < (int)nginxConfig._http.server.size(); i++) {
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
    Timer timer;
    try {
        while (true) {
            int result = kq.getEventsIndex();
            if (result == 0) {
                std::cout << "waiting..." << std::endl;
            } else {
                for (int i = 0; i < result; ++i) {
                    Socket* instance = reinterpret_cast<Socket*>(kq.getInstance(i));
                    long data = kq.getData(i);
                    if (dynamic_cast<KernelQueue::PairQueue*>(instance) != NULL) {
                        // NOTE: CGI Event 발생
                        KernelQueue::PairQueue* pairQueue = reinterpret_cast<KernelQueue::PairQueue*>(instance);
                        pairQueue->stopSlave();
                    } else if (dynamic_cast<ListeningSocket*>(instance) != NULL) {
                        // NOTE: Listening Socket Event 발생
                        // NOTE: 부하 테스트를 진행하며 여러 요청이 올 때 한번에 여러 연결을 생성하도록 하였습니다. 해당 구간에서 다음과 같이 수정함으로 성능 개선을 확인하였습니다.
                        std::cout << "[data : " << data << "]" << std::endl;
                        for (long i = 0; i < data; i++) {
                            ConnectionSocket* cSocket = new ConnectionSocket(instance->getSocket(), instance->getConfig(), nginxConfig);
                            timer.addObj(cSocket, std::atoi(instance->getConfig().dirMap["keepalive_timeout"].c_str()));
                            kq.addReadEvent(cSocket->getSocket(), reinterpret_cast<void*>(cSocket));
                        }
                    } else if (dynamic_cast<ConnectionSocket*>(instance) != NULL) {
                        // NOTE: Connection Socket Event 발생
                        ConnectionSocket* cSocket = dynamic_cast<ConnectionSocket*>(instance);
                        
                        if (kq.isClose(i)) {
                            // NOTE: 연결 종료 여부 확인
                            timer.delObj(cSocket, ConnectionSocket::ConnectionSocketKiller);
                            //delete cSocket;
                        } else if (kq.isReadEvent(i)) {
                            if (cSocket->HTTPRequestProcess() == HTTPRequestHandler::FINISH) {
                                // NOTE: to Write Event
                                kq.modEventToWriteEvent(i);
                            }
                        } else if (kq.isWriteEvent(i)) {
                            HTTPResponseHandler::Phase phase = cSocket->HTTPResponseProcess();
                            if (phase == HTTPResponseHandler::FINISH) {
                                kq.deletePairEvent(i);
                                timer.delObj(cSocket, ConnectionSocket::ConnectionSocketKiller);
                                //delete cSocket;
                            } else if (phase == HTTPResponseHandler::CGI_REQ) {
                                kq.setPairEvent(i, cSocket->getCGIfd());
                            }
                        }
                    }
                }
            }
            timer.CheckTimer(ConnectionSocket::ConnectionSocketKiller);
        }
    } catch (const std::exception &error) {
        std::cout << error.what() << std::endl;
        return (1);
    }
#endif
    return (0);
}