#include <iostream>
#include "Array.hpp"



int main(void) {
    Array<int> fds;

    int fd1 = 1;
    int fd2 = 2;
    int fd3 = 3;

    fds.appendElement(fd1);
    fds.appendElement(fd2);
    fds.appendElement(fd3);
    std::cout << "fds size: " << fds.size() << std::endl;

#if 1
    for (unsigned int i = 0; i < fds.size(); ++i) {
        std::cout << "i: " << i << std::endl;
        if (fds[i] == 2) {
            int tmp = i + 100;
            fds.appendElement(tmp);
        } else if (fds[i] == 1) {
            fds.removeElement(i);
            --i;
        }
        std::cout << "fds size: " << fds.size() << std::endl;
    }
#endif
      
    return (0);
}