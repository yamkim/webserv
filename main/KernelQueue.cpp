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

void* KernelQueue::getInstance(int index) {
    if (_pair.find(int(_getEvent[index].ident)) != _pair.end()) {
        _pair[int(_getEvent[index].ident)]->stopMaster();
    }
    return (_getEvent[index].udata);
}

long KernelQueue::getData(int index) {
    // NOTE: 필터의 특성에 대한 고유 데이터를 가져옴. (예를 들면 읽을수/쓸수 있는 버퍼 길이가 들어감.)
    if (_pair.find(int(_getEvent[index].ident)) != _pair.end()) {
        _pair[int(_getEvent[index].ident)]->stopMaster();
    }
    return (_getEvent[index].data);
}

KernelQueue::PairQueue::PairQueue(int kernelQueuefd) : Socket(-1) {
    _kernelQueuefd = kernelQueuefd;
    _master = NULL;
    _slave = NULL;
}

KernelQueue::PairQueue::~PairQueue() {
    delete _master;
    delete _slave;
}

void KernelQueue::PairQueue::setPairQueue(struct kevent master, struct kevent slave) {
    _master = new struct kevent(master);
    _slave = new struct kevent(slave);
}

void KernelQueue::setPairEvent(int masterIndex, int slaveReadFd) {
    struct kevent master;
    struct kevent slave;

    std::cout << "[EVENT - errcheck]" << std::endl;
    _pair[_getEvent[masterIndex].ident] = new KernelQueue::PairQueue(this->_kernelQueuefd);
    EV_SET( &master,
            _getEvent[masterIndex].ident,
            _getEvent[masterIndex].filter,
            _getEvent[masterIndex].flags,
            0,
            0,
            _getEvent[masterIndex].udata);
    EV_SET( &slave,
            slaveReadFd,
            EVFILT_READ,
            EV_ADD | EV_EOF,
            0,
            0,
            reinterpret_cast<void*>(_pair[_getEvent[masterIndex].ident]));
    _pair[_getEvent[masterIndex].ident]->setPairQueue(master, slave);
    // std::cout << "[DEBUG] : GEN " << _pair[_getEvent[masterIndex].ident] << std::endl;
    if (kevent(_kernelQueuefd, &slave, 1, NULL, 0, NULL) == -1) {
        throw ErrorHandler("Error: Kernel Queue setPairEvent Error.", ErrorHandler::CRITICAL, "KernelQueue::setPairEvent");
    }
    _pair[_getEvent[masterIndex].ident]->stopMaster();
}

void KernelQueue::deletePairEvent(int masterIndex) {
    if (_pair.find(int(_getEvent[masterIndex].ident)) != _pair.end()) {
        delete _pair[int(_getEvent[masterIndex].ident)];
        _pair.erase(int(_getEvent[masterIndex].ident));
    }
}

void KernelQueue::PairQueue::stopMaster(void) {
    struct kevent tmp[2];

    tmp[0] = *_master;
    tmp[1] = *_slave;
    tmp[0].flags = EV_DISABLE;
    tmp[1].flags = EV_ENABLE;
    if (kevent(_kernelQueuefd, tmp, 2, NULL, 0, NULL) == -1) {
        throw ErrorHandler("Error: Kernel Queue setPairEvent Error.", ErrorHandler::CRITICAL, "KernelQueue::setPairEvent");
    }
}

void KernelQueue::PairQueue::stopSlave(void) {
    struct kevent tmp[2];

    tmp[0] = *_master;
    tmp[1] = *_slave;
    tmp[0].flags = EV_ENABLE;
    tmp[1].flags = EV_DISABLE;
    if (kevent(_kernelQueuefd, tmp, 2, NULL, 0, NULL) == -1) {
        throw ErrorHandler("Error: Kernel Queue setPairEvent Error.", ErrorHandler::CRITICAL, "KernelQueue::setPairEvent");
    }
}

int KernelQueue::PairQueue::runSocket(void) {
    return (-1);
}
