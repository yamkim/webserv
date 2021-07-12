#ifndef HTTPRESPONSEHANDLER_HPP
#define HTTPRESPONSEHANDLER_HPP

#include <ctime>
#include <iostream>
#include <fcntl.h>

#include "FileController.hpp"
#include "HTTPHandler.hpp"
#include "CGISession.hpp"

#include "NginxConfig.hpp"
#include "HTMLBody.hpp"

#ifndef DEFAULT_ROOT
#define DEFAULT_ROOT std::string("/usr/share/nginx/html")
#endif
#ifndef WEBSERV_VERSION
#define WEBSERV_VERSION "0.0.0"
#endif

class HTTPResponseHandler : public HTTPHandler {
    private:
        HTTPResponseHandler();
    public:
        HTTPResponseHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, const NginxConfig::NginxConfig& nginxConf);
        virtual ~HTTPResponseHandler();

        typedef enum e_Phase {
            FIND_RESOURCE, 
            PRE_STATUSCODE_CHECK, 
            GET_STATIC_HTML, 
            GET_FILE, 
            REDIRECT, 
            CGI_RUN, 
            CGI_REQ, 
            DATA_SEND_LOOP, 
            CGI_SEND_LOOP, 
            CGI_RECV_HEAD_LOOP, 
            CGI_RECV_BODY_LOOP, 
            FINISH
        } Phase;
        virtual HTTPResponseHandler::Phase process(HTTPData& data, long bufferSize);


        int getCGIfd(void);
    private:
        std::string getMIME(const std::string& extension);
        bool isCGI(std::string& URI);
        // std::string getIndexPage(const std::string& absolutePath, std::vector<std::string>& indexVec);
        std::string getIndexPage(const std::string& absPath, const std::vector<std::string>& serverIndexVec, const std::vector<std::string>& locIndexVec);
        std::string getErrorPage(const std::string& absPath, const std::vector<std::string>& serverErrorPageVec, const std::vector<std::string>& locErrorPageVec);
        bool isErrorPageList(int statusCode, std::vector<std::string>& errorPageVec);
        void setGeneralHeader(HTTPData& data);
        void setHTMLHeader(const HTTPData& data);
        void showResponseInformation(HTTPData& data);

        void setCGIConfigMap();
        NginxConfig::LocationBlock getMatchingLocationConfiguration(const HTTPData& data);
        HTTPResponseHandler::Phase setInformation(HTTPData& data, int statusCode, const std::string& absPath);
 

    private:
        Phase _phase;
        FileController::Type _type;

        // root/data/index.html
        std::string _staticHtml;
        FileController* _file;
        CGISession* _cgi;

        NginxConfig::LocationBlock _locConf;
        std::string _indexPage;
        std::string _errorPage;
        std::string _CGIReceive;
        std::vector<std::string> _errorPageList;
        std::map<std::string, std::string> _cgiConfMap;
};

#endif
