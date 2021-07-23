#include "KernelQueue.hpp"

KernelQueue::KernelQueue(float pollingTime) {
    _kernelQueuefd = kqueue();
    if (_kernelQueuefd == -1) {
        throw ErrorHandler("Error: Kernel Queue Generate Error.", ErrorHandler::CRITICAL, "KernelQueue::KernelQueue");
    }
	_pollingTime.tv_sec = long(pollingTime);
	_pollingTime.tv_nsec = long(pollingTime * 1000000000L) % 1000000000L;
    _setEventIndex = 0;
}

KernelQueue::~KernelQueue() {
    close(_kernelQueuefd);
}

void KernelQueue::addEvent(int fd, int16_t event, void* instancePointer) {
    EV_SET(&_setEvent[_setEventIndex++], fd, event, EV_ADD | EV_EOF, 0, 0, instancePointer);
}

void KernelQueue::removeEvent(int fd, int16_t event, void* instancePointer) {
    EV_SET(&_setEvent[_setEventIndex++], fd, event, EV_DELETE, 0, 0, instancePointer);
}

int KernelQueue::getEventsIndex(void) {
    int eventCount = kevent(_kernelQueuefd, _setEvent, _setEventIndex, _getEvent, KERNELQUEUE_EVENTS_SIZE, &_pollingTime);
    if (eventCount == -1) {
        throw ErrorHandler("Error: Kernel Queue getEvents Error.", ErrorHandler::CRITICAL, "KernelQueue::getEvents");
    }
    _setEventIndex = 0;
    return (eventCount);
}

void KernelQueue::addReadEvent(int fd, void* instancePointer) {
    addEvent(fd, EVFILT_READ, instancePointer);
}

void KernelQueue::modEventToWriteEvent(int index) {
    removeEvent(_getEvent[index].ident, EVFILT_READ, _getEvent[index].udata);
    addEvent(_getEvent[index].ident, EVFILT_WRITE, _getEvent[index].udata);
}

void KernelQueue::modEventToReadEvent(int index) {
    removeEvent(_getEvent[index].ident, EVFILT_WRITE, _getEvent[index].udata);
    addEvent(_getEvent[index].ident, EVFILT_READ, _getEvent[index].udata);
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
    return (_getEvent[index].udata);
}

void KernelQueue::pairStopMaster(int index) {
    if (_pair.find(int(_getEvent[index].ident)) != _pair.end()) {
        _pair[int(_getEvent[index].ident)]->stopMaster();
    }
}

long KernelQueue::getData(int index) {
    return (_getEvent[index].data);
}

KernelQueue::PairQueue::PairQueue(int kernelQueuefd, struct kevent* setEvent, int& setEventIndex) : Socket(-1), _setEvent(setEvent), _setEventIndex(setEventIndex) {
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

void KernelQueue::setPairEvent(int masterIndex, int slaveReadFd, bool isRead) {
    struct kevent master;
    struct kevent slave;

    _pair[_getEvent[masterIndex].ident] = new KernelQueue::PairQueue(this->_kernelQueuefd, this->_setEvent, this->_setEventIndex);
    EV_SET( &master,
            _getEvent[masterIndex].ident,
            _getEvent[masterIndex].filter,
            EV_DISABLE,
            0,
            0,
            _getEvent[masterIndex].udata);
    EV_SET( &slave,
            slaveReadFd,
            isRead ? EVFILT_READ : EVFILT_WRITE,
            EV_ADD | EV_EOF,
            0,
            0,
            reinterpret_cast<void*>(_pair[_getEvent[masterIndex].ident]));
    _pair[_getEvent[masterIndex].ident]->setPairQueue(master, slave);
    _setEvent[_setEventIndex++] = master;
    _setEvent[_setEventIndex++] = slave;
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
    _setEvent[_setEventIndex++] = tmp[0];
    _setEvent[_setEventIndex++] = tmp[1];
}

void KernelQueue::PairQueue::stopSlave(void) {
    struct kevent tmp[2];

    tmp[0] = *_master;
    tmp[1] = *_slave;
    tmp[0].flags = EV_ENABLE;
    tmp[1].flags = EV_DISABLE;
    _setEvent[_setEventIndex++] = tmp[0];
    _setEvent[_setEventIndex++] = tmp[1];
}

int KernelQueue::PairQueue::runSocket(void) {
    return (-1);
}
