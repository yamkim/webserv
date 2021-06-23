#include "KernelQueue.hpp"

#include <iostream>

KernelQueue::KernelQueue() {
    _kernelQueuefd = kqueue();
    if (_kernelQueuefd == -1) {
        throw ErrorHandler("Error: Kernel Queue Generate Error.", ErrorHandler::CRITICAL, "KernelQueue::KernelQueue");
    }
    // FIXME: 시간 설정 다른데서 하는게 깔끔할듯
	// polling time = 1 sec
	_pollingTime.tv_sec = 1;
	_pollingTime.tv_nsec = 0;
}

KernelQueue::~KernelQueue() {
    close(_kernelQueuefd);
}

void KernelQueue::addEvent(int fd, int16_t event, void* instancePointer) {
    EV_SET(&_eventSetting, fd, event, EV_ADD | EV_EOF, 0, 0, instancePointer);
    if (kevent(_kernelQueuefd, &_eventSetting, 1, NULL, 0, NULL) == -1) {
        throw ErrorHandler("Error: Kernel Queue addevent Error.", ErrorHandler::CRITICAL, "KernelQueue::addEvent");
    }
}

void KernelQueue::removeEvent(int fd, int16_t event, void* instancePointer) {
    EV_SET(&_eventSetting, fd, event, EV_DELETE, 0, 0, instancePointer);
    if (kevent(_kernelQueuefd, &_eventSetting, 1, NULL, 0, NULL) == -1) {
        throw ErrorHandler("Error: Kernel Queue removeEvent Error.", ErrorHandler::CRITICAL, "KernelQueue::addEvent");
    }
}

int KernelQueue::getEventsIndex(void) {
    int eventCount = kevent(_kernelQueuefd, NULL, 0, _getEvent, KERNELQUEUE_EVENTS_SIZE, &_pollingTime);
    if (eventCount == -1) {
        throw ErrorHandler("Error: Kernel Queue getEvents Error.", ErrorHandler::CRITICAL, "KernelQueue::getEvents");
    }
    return (eventCount);
}

void KernelQueue::addReadEvent(int fd, void* instancePointer) {
    addEvent(fd, EVFILT_READ, instancePointer);
}

void KernelQueue::modEventToWriteEvent(int index) {
    removeEvent(_getEvent[index].ident, EVFILT_READ, _getEvent[index].udata);
    addEvent(_getEvent[index].ident, EVFILT_WRITE, _getEvent[index].udata);
}

bool KernelQueue::isClose(int index) const {
    return (_getEvent[index].flags & EV_EOF);
}

bool KernelQueue::isReadEvent(int index) const {
    return (_getEvent[index].filter == EVFILT_READ);
}

bool KernelQueue::isWriteEvent(int index) const {
    return (_getEvent[index].filter == EVFILT_WRITE);
}

int KernelQueue::getFd(int index) const {
    return (_getEvent[index].ident);
}

void* KernelQueue::getInstance(int index) const {
    return (_getEvent[index].udata);
}
