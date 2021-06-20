#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include <string>
#include <map>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "ErrorHandler.hpp"

#include <iostream> // NOTE : 디버깅용 (추후에 제거 예정)

// REVIEW : 읽어오는데 사용할 버퍼 크기는 클래스 내부 버퍼 (char 배열) 크기를 정의하는데 사용되는데 이렇게 사용하는것보다 더 좋은 방법이 있으면 좋겟군요... static const로 내부 버퍼 크기를 정의하면 비표준인 VLA로 정의하게 되는지 검토 필요합니다.
#define REQUEST_BUFFER_SIZE 100

class HTTPRequestHandler {
    // NOTE : 추후에 추가해야 할 기능이 늘어나거나 줄어들 수 있어 불가피하게 기능별로 나눠놨습니다. 추후에 간결하게 작성할 예정입니다.
    public:
        typedef enum e_Method {UNDEF, GET, POST, DELETE} Method;
    private:
        Method _method;
        std::string _URI;
        std::map<std::string, std::string> _headers;
    private:
        typedef enum e_Phase {PARSESTARTLINE, PARSEHEADER, OK, FAIL, CONNECTIONCLOSE} Phase;
        Phase _phase;
        int _connectFd;
        std::string _buffer;
        bool _finish;
        HTTPRequestHandler();
    public:
        HTTPRequestHandler(int connectFd);
        ~HTTPRequestHandler();
        void process(void);
        bool isFinish(void);
        bool isConnectionCloseByClient(void);
    public: // REVIEW getter로 파싱된 항목 가지고 오게 하는게 좋을지 더 좋은 방법이 있는지 고민 중입니다.
        Method getMethod(void) const;
        const std::string& getURI(void) const;
        const std::map<std::string, std::string>& getHeaders(void) const;
    private:
        std::string getStringHeadByDelimiter(const std::string &buf, std::size_t &pos, const std::string &needle);
        bool readBufferTillNewLine(void);
        int findNewLine(const char *buffer);
        bool getHeaderStartLine(void);
        bool getHeader(void);
};

#endif
