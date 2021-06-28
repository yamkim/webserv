#include "PairArray.hpp"

PairArray::PairArray() {
    this->_arr = new struct pollfd[0]();
    this->_size = 0;
}

PairArray::~PairArray() {
    delete [] _arr;
}

std::pair<Socket*, int>& PairArray::operator[](unsigned int i) {
	return (this->_vec[i]);
}

size_t PairArray::getSize() const {
    return (_vec.size());
}

struct pollfd* PairArray::getArray() {
    delete [] this->_arr;
    this->_arr = new struct pollfd[this->_vec.size()]();
    for (size_t i = 0; i < this->_vec.size(); ++i) {
        this->_arr[i] = this->_vec[i].first->getPollFd();
    }
    return (this->_arr);
}

std::vector<std::pair<Socket*, int> > PairArray::getVector() const {
    return (this->_vec);
}

void PairArray::renewVector() {
    for (size_t i = 0; i < this->_vec.size(); ++i) {
        this->_vec[i].first->setPollFd(this->_arr[i]);
    }
}

void PairArray::appendElement(Socket* socket_, int type) {
    _vec.push_back(std::make_pair(socket_, type));
    _size = _vec.size();
}

void PairArray::removeElement(size_t idx) {
    _vec.erase(_vec.begin() + idx);
    _size = _vec.size();
}

void PairArray::showVector() {
    if (_vec.empty())
        return ;
    for (size_t i = 0; i < _vec.size(); ++i) {
        std::cout << "vec[" << i << "]: " << *(_vec[i].first) << std::endl;
        std::cout << "vec[" << i << "] type: " << _vec[i].second << std::endl;
    }
    std::cout << std::endl;
}

void PairArray::showArray() {
    if (_vec.empty())
        return ;
    _arr = getArray();
    for (size_t i = 0; i < _vec.size(); ++i) {
        std::cout << "arr[" << i << "] fd: " << _arr[i].fd << std::endl;
    }
    std::cout << std::endl;
}