#ifndef KERNELQUEUE_HPP
#define KERNELQUEUE_HPP

#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include "ErrorHandler.hpp"
#include "Socket.hpp"

#define KERNELQUEUE_EVENTS_SIZE 10

class KernelQueue {
    private:
        int _kernelQueuefd;
        struct kevent _eventSetting;
        struct kevent _getEvent[KERNELQUEUE_EVENTS_SIZE];
        struct timespec _pollingTime;
        void addEvent(int fd, int16_t event, void* instancePointer);
        void removeEvent(int fd, int16_t event, void* instancePointer);
        KernelQueue();
    public:
        KernelQueue(float pollingTime);
        ~KernelQueue();
        int getEventsIndex(void);
        void addReadEvent(int fd, void* instancePointer);
        void modEventToWriteEvent(int index);
        bool isClose(int index) const;
        bool isReadEvent(int index) const;
        bool isWriteEvent(int index) const;
        int getFd(int index) const;
        void* getInstance(int index);
        long getData(int index);
        void setPairEvent(int masterIndex, int slaveReadFd);
        void deletePairEvent(int masterIndex);
        class PairQueue : public Socket {
            private:
                int _kernelQueuefd;
                struct kevent* _master;
                struct kevent* _slave;
            public:
                PairQueue(int kernelQueuefd);
                virtual ~PairQueue();
                void setPairQueue(struct kevent master, struct kevent slave);
                void stopMaster(void);
                void stopSlave(void);
                virtual int runSocket(void);
        };
    private:
        std::map<int, PairQueue*> _pair;
};

#endif
