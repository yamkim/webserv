#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include <cstring>
#include <unistd.h>
#include "HTTPHandler.hpp"

#include <iostream> // NOTE : 디버깅용 (추후에 제거 예정)

class HTTPRequestHandler : public HTTPHandler {
    // NOTE : 추후에 추가해야 할 기능이 늘어나거나 줄어들 수 있어 불가피하게 기능별로 나눠놨습니다. 추후에 간결하게 작성할 예정입니다.
    private:
        HTTPRequestHandler();
    public:
        typedef enum e_Phase {PARSE_STARTLINE, PARSE_HEADER, CONNECTION_CLOSE, FINISH} Phase;
        Phase _phase;
        HTTPRequestHandler(int connectionFd);
        virtual ~HTTPRequestHandler();
        virtual HTTPRequestHandler::Phase process();
    public: // REVIEW getter로 파싱된 항목 가지고 오게 하는게 좋을지 더 좋은 방법이 있는지 고민 중입니다.
        Method getMethod(void) const;
        const std::string& getURI(void) const;
        const std::map<std::string, std::string>& getHeaders(void) const;
    private:
        std::string getStringHeadByDelimiter(const std::string &buf, std::size_t &pos, const std::string &needle);
        bool setHeaderString(void);
        void setMethod(void);
        void setURI(void);
        void setProtocol(void);
        int findNewLine(const char *buffer);
        bool getHeaderStartLine(void);
        bool getHeader(void);
};

#endif
