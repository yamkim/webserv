#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <map>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "Socket.hpp"

class KQueue {
    public:
        std::map<uintptr_t, Socket*> _sockets;
        struct timespec _timeout;
        struct kevent _event;
        int _kqueue;
        struct kevent getEvent[10];
        
        KQueue() {
            _kqueue = kqueue();

            _timeout.tv_sec = 1;
            _timeout.tv_nsec = 0;
        }

        void addEvent(Socket* addSocket) {
            EV_SET(&_event, addSocket->getSocket(), EVFILT_READ, EV_ADD | EV_EOF, 0, 0, NULL);
            if (kevent(_kqueue, &_event, 1, NULL, 0, NULL) == -1) {
                throw ErrorHandler("Error: Kernel Queue kevent Error.", ErrorHandler::CRITICAL, "KernelQueue::addEvent");
            }
            std::cout << "add socket: " << addSocket->getSocket() << std::endl;
            _sockets[addSocket->getSocket()] = addSocket;
        }

        void deleteEvent(int fd) {
            EV_SET(&_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            if (kevent(_kqueue, &_event, 1, NULL, 0, NULL) == -1) {
                throw ErrorHandler("Error: Kernel Queue kevent Error.", ErrorHandler::CRITICAL, "KernelQueue::deleteEvent");
            }
            close(fd); // Socket class 소멸자
        }

        int run(struct kevent* changeList) {
            int ret = kevent(_kqueue, NULL, 0, changeList, 10, &_timeout);
            if (ret == -1) {
                throw ErrorHandler("Error: kevent error.", ErrorHandler::CRITICAL, "KernelQueue::addEvent");
            } else if (ret == 0) {
                std::cout << "waiting..." << std::endl;
            }
            return ret;
        }

        Socket* getInstance(uintptr_t fd) {
            return (_sockets[fd]);
        }
};

#endif