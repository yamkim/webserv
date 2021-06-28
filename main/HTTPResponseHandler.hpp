#ifndef HTTPRESPONSEHANDLER_HPP
#define HTTPRESPONSEHANDLER_HPP

#include <ctime>
#include <sstream>
#include <iostream>
#include <fcntl.h>

#include "FileController.hpp"
#include "HTTPHandler.hpp"
#include "CGISession.hpp"

#include "NginxConfig.hpp"

class HTTPResponseHandler : public HTTPHandler {
    private:
        HTTPResponseHandler();
    public:
        HTTPResponseHandler(int coneectionFd, std::string arg);
        virtual ~HTTPResponseHandler();

        typedef enum e_Phase {FIND_RESOURCE, AUTOINDEX, CGI_RUN, CGI_REQ, GET_FILE, NOT_FOUND, DATA_SEND_LOOP, CGI_SEND_LOOP, FINISH} Phase;
        virtual HTTPResponseHandler::Phase process(void);
        int getCGIfd(void);
    private:
        void setGeneralHeader(std::string status);
        void setTypeHeader(std::string type);
        void setLengthHeader(long contentLength);
        void convertHeaderMapToString(bool isCGI);
        static std::string get404Body(void);
        static std::string getAutoIndexBody(std::string root, std::string path);
        std::string getMIME(std::string& extension);
        std::string getExtenstion(std::string& URI);
        bool isCGI(std::string& URI);
        std::string getIndex(void);
    private:
        Phase _phase;
        std::string _root;
        std::string _internalHTMLString;
        std::string _absolutePath;
        std::string _extension;
        std::string _staticHtml;
        FileController* _file;
        CGISession _cgi;
        NginxConfig _nginxConfig;
};

#endif
