#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include <cstring>
#include <unistd.h>
#include <ctime>
#include "HTTPHandler.hpp"
#include "FileController.hpp"
#include "Parser.hpp"

#include <iostream> // NOTE : 디버깅용 (추후에 제거 예정)

class HTTPRequestHandler : public HTTPHandler {
    private:
        HTTPRequestHandler();
    // NOTE : 추후에 추가해야 할 기능이 늘어나거나 줄어들 수 있어 불가피하게 기능별로 나눠놨습니다. 추후에 간결하게 작성할 예정입니다.
    public:
        HTTPRequestHandler(int connectionFd, const NginxConfig& nginxConf);
        virtual ~HTTPRequestHandler();

        typedef enum e_Phase {PARSE_STARTLINE, PARSE_HEADER, PARSE_BODY, FINISH} Phase;
        virtual HTTPRequestHandler::Phase process(HTTPData& data);
    public: // REVIEW getter로 파싱된 항목 가지고 오게 하는게 좋을지 더 좋은 방법이 있는지 고민 중입니다.
    private:
        bool getRequestLine(HTTPData& data);
        bool getHeader(void);

        int findNewLine(const char *buffer);
        bool setHeaderString(void);

        void setMethod(std::string method);
        void setURI(std::string URI);
        void setProtocol(std::string protocol);

        std::string _requestLine;
        Phase _phase;
        long _contentLength;
        FileController* _fileController;

};

#endif
