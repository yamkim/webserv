#ifndef HTTPRESPONSEHANDLER_HPP
#define HTTPRESPONSEHANDLER_HPP

#include <ctime>
#include <iostream>
#include <fcntl.h>

#include "FileController.hpp"

#include <string>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include "ErrorHandler.hpp"
#include "HTTPData.hpp"

#include "CGISession.hpp"

#include "NginxConfig.hpp"
#include "HTMLBody.hpp"

#define RESPONSE_BUFFER_SIZE 1000

class HTTPResponseHandler {
    private:
        HTTPResponseHandler();
    public:
        HTTPResponseHandler(int coneectionFd);
        virtual ~HTTPResponseHandler();

        typedef enum e_Phase {FIND_RESOURCE, AUTOINDEX, CGI_RUN, CGI_REQ, GET_FILE, NOT_FOUND, DATA_SEND_LOOP, CGI_SEND_LOOP, CGI_RECV_LOOP, FINISH} Phase;
        virtual HTTPResponseHandler::Phase process(HTTPData& data);

        void responseNotFound(const HTTPData& data);
        void responseAutoIndex(const HTTPData& data);

        int getCGIfd(void);
    private:
        std::string getMIME(const std::string& extension) const;
        bool isCGI(std::string& URI);
        std::string getServerIndex(NginxConfig::ServerBlock server);
        void setHTMLHeader(const std::string& extension, const long contentLength);
        void setGeneralHeader(std::string status);
        void setTypeHeader(std::string type);
        void setLengthHeader(long contentLength);
        void convertHeaderMapToString(bool isCGI);

    private:
        Phase* _phase;
        std::string* _root;
        std::string* _headerString;
        std::string* _serverIndex;
        std::string* _absolutePath;
        std::string* _staticHtml;

        FileController::Type* _type;
        FileController** _file;
        CGISession** _cgi;
        NginxConfig* _nginxConfig;
        std::map<std::string, std::string>* _headers;
        int* _connectionFd;
};

#endif
