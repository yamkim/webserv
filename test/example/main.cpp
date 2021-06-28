#include <sys/event.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

int main(int argc, char **argv) {
    struct	kevent event;	 /* Event we want to monitor */
    struct	kevent tevent;	 /* Event triggered */
    int kq, fd, ret;

    if (argc != 2) 
        err(EXIT_FAILURE, "Usage: %s path\n", argv[0]);
    // 관측하기 위한 file descriptor를 생성
    if ((fd = open(argv[1], O_RDONLY)) == -1)
        err(EXIT_FAILURE, "Failed to open '%s'", argv[1]);

    /* Create kqueue. */
    kq = kqueue();
    if (kq == -1)
        err(EXIT_FAILURE, "kqueue() failed");

    /* Initialize kevent structure. */
    // - 파일 포인터가 eof가 아닌 경우 반환합니다.
    // - data는 eof로부터의 offset을 포함합니다. 
    // - ident: 식별하기 위한 값으로, 
    // EVFILT_READ: FD를 ident로 지정하여 읽을 data가 생길 때마다 반환합니다.
    // EVFILT_WRITE: FD를 ident로 지정하여 쓸 data가 생길 때마다 반환합니다.
    // EVFILT_EMPTY: FD를 ident로 지정하여 쓸 data가 없을 때마다 반환합니다.
    // EVFILT_VNODE: fflags에서 지정한 event를 ident로 지정하여 event 발생 시에 반환합니다.
    // EVFILT_PROC: fflags에서 지정한 event를 ident로 지정하여 event 발생 시에 반환합니다.
    // EVFILT_TIMER: 임의의 타이머를 ident로 지정하여 주기마다 반환합니다.

    // event 관측하고자 하는 kevent를 fd에 대해서 관측하고, event가 발생할 때마다 반환합니다.
    // 관측하던 fd가 수정되면, 기존의 event의 변수를 수정합니다. EV_DISABLE 플레그에 의해 덮어씌어지지 않는다면, 자동적으로 이벤트를 enable 합니다.
    EV_SET(&event, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, EVFILT_READ | EVFILT_WRITE, 0, NULL);
    // EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    /* Attach event to the	kqueue.	*/
    // event를 다시 등록하지 않더라도, kq에 기억하고 있다가 호출시에 호출합니다.
    ret = kevent(kq, &event, 1, NULL, 0, NULL);
    if (ret == -1)
        err(EXIT_FAILURE, "kevent register");
    if (event.flags & EV_ERROR)
        errx(EXIT_FAILURE,	"Event error: %s", strerror(event.data));

    // #define EV_LENGTH 5
    // struct kevent ke[EV_LENGTH];
    
    for (;;) {
        /*	Sleep until something happens. */
        ret = kevent(kq, NULL, 0, &tevent,	1, NULL);
        // ke에 등록된 애들에 플래그가 발생하면 추적
        // ret = kevent(kq, NULL, 0, ke, EV_LENGTH, NULL);
        std::cout << "ident: " << tevent.ident << std::endl;
        std::cout << "data: " << tevent.data << std::endl;
        std::cout << "flags: " << tevent.flags << std::endl;
        std::cout << "fflags: " << tevent.fflags << std::endl;
        std::cout << "ret: " << ret << std::endl;
        if	(ret ==	-1) {
            err(EXIT_FAILURE, "kevent wait");
        } else if (ret > 0) {
            if ((int)tevent.fflags == NOTE_ATTRIB) {
                std::cout << "NOTE_ATTRIB case" << std::endl;
            } else if ((int)tevent.fflags == (NOTE_RENAME | NOTE_DELETE)) {
                std::cout << "NOTE_RENAME | NOTE_DELETE case" << std::endl;
            } else if ((int)tevent.fflags == NOTE_WRITE) {
                std::cout << "NOTE_WRITE case" << std::endl;
            } else if ((int)tevent.fflags == (NOTE_LINK | NOTE_DELETE)) {
                std::cout << "NOTE_LINK | NOTE_DELETE case" << std::endl;
            } else if ((int)tevent.fflags == (NOTE_NONE)) {
                std::cout << "NOTE_NONE case" << std::endl;
            }
            
            printf("Something was written in '%s'\n", argv[1]);
        }
    }
}
