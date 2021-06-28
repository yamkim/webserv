#include <iostream>
#include <cstring>
#include <string>
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include "KQueue.hpp"

// kqueue
#include <sys/event.h>
#include <sys/time.h>

// kqueue 전반적인 설명: https://incredible-larva.tistory.com/entry/IO-Multiplexing-%ED%86%BA%EC%95%84%EB%B3%B4%EA%B8%B0-2%EB%B6%80
// kqueue 예시: https://www.freebsd.org/cgi/man.cgi?kqueue

std::string genHttpHeader() {
    std::string ret;
    // HTTP Header 및 Body
    ret += "HTTP/1.1 200 OK\r\n";
    ret += "Content-Type: text/html\r\n";
    ret += "\r\n";
    ret += "<html>";
    ret += "<head><title>hi</title></head>";
    ret += "<body>";
    ret += "<b><center> HI! </center></b>";
    ret += "</body>";
    ret += "</html>";
    return ret;
}

int main(void) {
    KQueue kq;

    try {
        ListeningSocket* lSocket4200 = new ListeningSocket(4200, 42);
        if (lSocket4200->runSocket())
            return (1);
        kq.addEvent(lSocket4200);

        ListeningSocket* lSocket4242 = new ListeningSocket(4242, 42);
        if (lSocket4242->runSocket())
            return (1);
        kq.addEvent(lSocket4242);

        while (true) {
            struct kevent changeList[10];
            int result = kq.run(changeList);
            for (int i = 0; i < result; i++) {
                Socket* curSocket = kq.getInstance(changeList[i].ident);
                if (dynamic_cast<ListeningSocket*>(curSocket)) {
                    ConnectionSocket* cSocket = new ConnectionSocket(curSocket->getSocket());
                    kq.addEvent(cSocket);
                } else {
                    char buffer[1024];
                    int readLength;

                    readLength = read(changeList[i].ident, buffer, 1024);
                    std::cout << "data : " << std::string(buffer, readLength) << std::endl;

                    std::string httpTestHeaderString = genHttpHeader();
                    write(changeList[i].ident, httpTestHeaderString.data(), httpTestHeaderString.length());
                    kq.deleteEvent(changeList[i].ident);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
	return (0);
}

                // std::cout << "getEvent  [" << i << "].ident: " << changeList[i].ident << std::endl;
                // std::cout << "changeList[" << i << "].filter: " << changeList[i].filter << std::endl;
                // std::cout << "changeList[" << i << "].flags: " << changeList[i].flags << std::endl;
                // std::cout << "changeList[" << i << "].fflags: " << changeList[i].fflags << std::endl;
                // std::cout << "changeList[" << i << "].data: " << changeList[i].data << std::endl;