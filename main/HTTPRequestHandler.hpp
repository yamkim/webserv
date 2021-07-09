#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include <cstring>
#include <unistd.h>
#include <ctime>
#include "HTTPHandler.hpp"
#include "FileController.hpp"
#include "Parser.hpp"
#include <iostream>

class HTTPRequestHandler : public HTTPHandler {
    public:
        HTTPRequestHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, NginxConfig& nginxConf);
        virtual ~HTTPRequestHandler();
        typedef enum e_Phase {PARSE_STARTLINE, PARSE_HEADER, PARSE_BODY, FINISH} Phase;
        virtual HTTPRequestHandler::Phase process(HTTPData& data);
    private:
        bool getStartLine(HTTPData& data);
        std::string _requestLine;
        Phase _phase;
        long _contentLength;
        FileController* _fileController;
        HTTPRequestHandler();

        bool getHeader(void);
        int findNewLine(const char *buffer);
        bool setHeaderString(void);
        void setMethod(std::string method);
        void setURI(std::string URI);
        void setProtocol(std::string protocol);
};

#endif
