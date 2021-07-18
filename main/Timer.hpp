#ifndef TIMER_HPP
#define TIMER_HPP

#include <vector>
#include <sys/time.h>

class Timer {
    private:
        typedef struct s_Pair {
            void* obj;
            long genTime;
            long expTime;
        } Pair;
        std::vector<Pair> _timerList;
    public:
        Timer();
        ~Timer();
        void addObj(void* obj, int lifetime);
        void resetObj(void* obj, int lifetime);
        void delObj(void* obj, void (*del)(void*));
        void CheckTimer(void (*del)(void*));
};


#endif
