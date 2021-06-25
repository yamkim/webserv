#include <iostream>
//#include <cstring>
//#include <string>
//#include <arpa/inet.h>
// #include <sys/socket.h>
// #include <sys/errno.h>
// #include <sys/time.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <poll.h>
#include "PairArray.hpp"
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include <vector>
#include "KernelQueue.hpp"

int main(void)
{
    KernelQueue kq;
#if 1
    // NOTE: 여러 개의 소켓 관리도 간편하게 가능
    ListeningSocket* lSocket4200 = new ListeningSocket(4200, 42);
    if (lSocket4200->runSocket())
        return (1);
    lSocket4200->setPollFd(POLLIN);
    kq.addReadEvent(lSocket4200->getSocket(), reinterpret_cast<void*>(lSocket4200));
    ListeningSocket* lSocket8080 = new ListeningSocket(8080, 42);
    if (lSocket8080->runSocket())
        return (1);
    lSocket8080->setPollFd(POLLIN);
    kq.addReadEvent(lSocket8080->getSocket(), reinterpret_cast<void*>(lSocket8080));
    //pollfds.appendElement(lSocket1, PairArray::LISTENING);
#endif

#if 0
    Socket* lSocket1 = new ListeningSocket(4201, 42);
    if (lSocket1->runSocket())
        return (1);
    Socket* lSocket2 = new ListeningSocket(4202, 42);
    if (lSocket2->runSocket())
        return (1);
    Socket* lSocket3 = new ListeningSocket(4203, 42);
    if (lSocket3->runSocket())
        return (1);
    
    lSocket1->setPollFd(POLLIN);
    lSocket2->setPollFd(POLLIN);
    lSocket3->setPollFd(POLLIN);

    pollfds.appendElement(lSocket1, PairArray::LISTENING);
    pollfds.appendElement(lSocket2, PairArray::LISTENING);
    pollfds.appendElement(lSocket3, PairArray::LISTENING);
 #endif

    try {
        while (true) {
            //TODO: joopark - 커널큐로 테스트 해보기 (코드 반영 x)
            //int result = poll(pollfds.getArray(), pollfds.getSize(), 1000);
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
                            if (cSocket->HTTPRequestProcess() == HTTPRequestHandler::FINISH) {
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
    return (0);
}
    
