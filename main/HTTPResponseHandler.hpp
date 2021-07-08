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

class HTTPResponseHandler : public HTTPHandler {
    private:
        HTTPResponseHandler();
    public:
        HTTPResponseHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, const NginxConfig& nginxConf);
        virtual ~HTTPResponseHandler();

        typedef enum e_Phase {
            FIND_RESOURCE, 
            GET_STATIC_HTML, 
            GET_FILE, 
            REDIRECT, 
            CGI_RUN, 
            CGI_REQ, 
            DATA_SEND_LOOP, 
            CGI_SEND_LOOP, 
            CGI_RECV_LOOP, 
            FINISH
        } Phase;
        virtual HTTPResponseHandler::Phase process(HTTPData& data, long bufferSize);

        void responseNotFound(const HTTPData& data);
        void responseAutoIndex(const HTTPData& data);
        void responseTest(const HTTPData& data);

        int getCGIfd(void);
    private:
        std::string getMIME(const std::string& extension) const;
        bool isCGI(std::string& URI);
        // std::string getIndexPage(const std::string& absolutePath, std::vector<std::string>& indexVec);
        std::string getIndexPage(const HTTPData& data, const std::vector<std::string>& serverIndexVec, const std::vector<std::string>& locIndexVec);
        std::string getErrorPage(const HTTPData& data, const std::vector<std::string>& serverErrorPageVec, const std::vector<std::string>& locErrorPageVec);
        // std::string getErrorPage(const std::string& absolutePath, std::vector<std::string>& errorPageVec);
        bool isErrorPageList(int statusCode, std::vector<std::string>& errorPageVec);
        void setGeneralHeader(int status);
        void setHTMLHeader(const HTTPData& data);
        void showResponseInformation(HTTPData& data);
        HTTPResponseHandler::Phase setError(HTTPData& data);
 

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
        std::vector<std::string> _errorPageList;
        std::map<std::string, std::string> _cgiConfMap;
};

#endif
