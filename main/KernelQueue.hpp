#ifndef KERNELQUEUE_HPP
#define KERNELQUEUE_HPP

#include <map>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <unistd.h> // for close()
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
        void* getInstance(int index);
        long getData(int index);
        void setPairEvent(int masterIndex, int slaveReadFd); // NOTE: 현재 호출된 이벤트의 인덱스와 새로 추가할 fd를 인수로 넣으면 자동으로 쌍이 생성되고 이벤트가 추가된다.
        void deletePairEvent(int masterIndex); // NOTE: 이벤트 쌍을 제거한다.
        // REVIEW: 이벤트의 대한 처리는 최대한 커널큐 내부에서 하기 위해 Nested Class 형식을 사용하였고 다형성을 위해 Socket 클래스를 상속 받습니다.
        class PairQueue : public Socket {
            private:
                int _kernelQueuefd; // REVIEW: Socket 클래스의 _socket을 사용하면 파괴자에서 닫히는 문제와 명칭 때문에 커널큐 fd를 별도로 설정했습니다.
                struct kevent* _master; // NOTE: HTTP Response Event
                struct kevent* _slave; // NOTE: CGI Output Event
            public:
                PairQueue(int kernelQueuefd);
                virtual ~PairQueue();
                void setPairQueue(struct kevent master, struct kevent slave);
                void stopMaster(void); // NOTE: 호출 시 HTTP Response 이벤트를 잠시 중단한다.
                void stopSlave(void); // NOTE: 호출 시 CGI Output 이벤트를 잠시 중단한다.
                virtual int runSocket(void); // NOTE: 부모 클래스의 runSocket를 사용하지 않으므로 아무 동작도 하지 않는다.
        };
    private:
        std::map<int, PairQueue*> _pair; // NOTE: HTTP Response - CGI 쌍은 커널큐 내에서 관리한다.
};

#endif
