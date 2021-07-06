#include "Timer.hpp"

Timer::Timer() {
}

Timer::~Timer() {
}

void Timer::addObj(void* obj, int lifetime) {
    struct timeval time;
    Pair pair;

    gettimeofday(&time, NULL);
    pair.obj = obj;
    pair.genTime = time.tv_sec;
    pair.expTime = time.tv_sec + long(lifetime);
    _timerList.push_back(pair);
}

void Timer::delObj(void* obj, void (*del)(void*)) {
    std::vector<Pair>::iterator iter;
    for (iter = _timerList.begin(); iter != _timerList.end();) {
        if ((*iter).obj == obj) {
            del((*iter).obj);
            iter = _timerList.erase(iter);
        } else {
            iter++;
        }
    }
}

void Timer::CheckTimer(void (*del)(void*)) {
    struct timeval time;
    gettimeofday(&time, NULL);
    std::vector<Pair>::iterator iter;
    for (iter = _timerList.begin(); iter != _timerList.end();) {
        if ((*iter).expTime < time.tv_sec) {
            del((*iter).obj);
            iter = _timerList.erase(iter);
        } else {
            iter++;
        }
    }
}
