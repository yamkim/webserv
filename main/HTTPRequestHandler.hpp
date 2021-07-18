#ifndef HTTPREQUESTHANDLER_HPP
#define HTTPREQUESTHANDLER_HPP

#include <cstring>
#include <unistd.h>
#include <ctime>
#include "HTTPHandler.hpp"
#include "FileController.hpp"
#include "Parser.hpp"
#include "Utils.hpp"
#include <iostream>

class HTTPRequestHandler : public HTTPHandler {
    public:
        HTTPRequestHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, NginxConfig::NginxConfig& nginxConf);
        virtual ~HTTPRequestHandler();
        typedef enum e_Phase {PARSE_STARTLINE, PARSE_HEADER, PARSE_BODY_NBYTES, PARSE_BODY_CHUNK, REMOVE_CRNF, FINISH} Phase;
        virtual HTTPRequestHandler::Phase process(HTTPData& data, long bufferSize);
    private:
        bool getStartLine(HTTPData& data);
        std::string _stringBuffer;
        bool _stringBufferClear;
        Phase _phase;
        long _contentLength;
        long _contentLengthSum;
        bool _chunkFinish;
        long _dynamicBufferSize;
        FileController* _fileController;
        HTTPRequestHandler();

        bool getHeader(void);
        std::string* getDataByCRNF(int searchLength);
        void setMethod(std::string method);
        void setURI(std::string URI);
        void setProtocol(std::string protocol);
};

#endif
