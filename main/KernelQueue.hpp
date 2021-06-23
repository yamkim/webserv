#ifndef KERNELQUEUE_HPP
#define KERNELQUEUE_HPP

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <unistd.h> // for close()
#include "ErrorHandler.hpp"

#define KERNELQUEUE_EVENTS_SIZE 10

class KernelQueue {
    private:
        int _kernelQueuefd;
        struct kevent _eventSetting;
        struct kevent _getEvent[KERNELQUEUE_EVENTS_SIZE];
        struct timespec _pollingTime;
        void addEvent(int fd, int16_t event, void* instancePointer);
        void removeEvent(int fd, int16_t event, void* instancePointer);
    public:
        KernelQueue();
        ~KernelQueue();
        int getEventsIndex(void);
        void addReadEvent(int fd, void* instancePointer);
        void modEventToWriteEvent(int index);
        bool isClose(int index) const;
        bool isReadEvent(int index) const;
        bool isWriteEvent(int index) const;
        int getFd(int index) const;
        void* getInstance(int index) const;
};

#endif
