#ifndef PAIRARRAY_HPP
#define PAIRARRAY_HPP

#include <vector>
#include "Socket.hpp"

class PairArray {
    private:
        unsigned int _size;
        struct pollfd *_arr;
        std::vector<std::pair<Socket*, int> > _vec;
        
    public:
        PairArray();
        ~PairArray();
        std::pair<Socket*, int>& operator[](unsigned int i);

        typedef enum e_SocketType {NONE, LISTENING, CONNECTION} SocketType;
        size_t getSize() const;
        struct pollfd* getArray();
        std::vector<std::pair<Socket*, int> > getVector() const;

        void renewVector();
        void appendElement(Socket* socket_, int type);
        void removeElement(size_t pos);
        void showVector();
        void showArray();
};

#endif