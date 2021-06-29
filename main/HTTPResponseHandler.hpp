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
        std::string _serverIndex;
    public:
        HTTPResponseHandler(int coneectionFd, std::string arg);
        virtual ~HTTPResponseHandler();

        typedef enum e_Phase {FIND_RESOURCE, AUTOINDEX, CGI_RUN, CGI_REQ, GET_FILE, NOT_FOUND, DATA_SEND_LOOP, CGI_SEND_LOOP, FINISH} Phase;
        virtual HTTPResponseHandler::Phase process(HTTPHandler::ConnectionData& data);

        void responseNotFound(void);
        void responseAutoIndex(void);

        int getCGIfd(void);
    private:
        static std::string get404Body(void);
        static std::string getAutoIndexBody(std::string root, std::string path);
        std::string getMIME(const std::string& extension) const;
        bool isCGI(std::string& URI);
        std::string getServerIndex(NginxConfig::ServerBlock server);
        void setHTMLHeader(const std::string& extension, const long contentLength);

    private:
        Phase _phase;
        std::string _root;
        FileController::Type _type;

        std::string _internalHTMLString;
        std::string _absolutePath;
        std::string _extension;
        std::string _staticHtml;
        FileController* _file;
        CGISession* _cgi;
        NginxConfig _nginxConfig;
};

#endif
