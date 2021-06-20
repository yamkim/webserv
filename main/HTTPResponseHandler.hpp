#ifndef HTTPRESPONSEHANDLER_HPP
#define HTTPRESPONSEHANDLER_HPP

#include <unistd.h>
#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <iostream>

#include "ErrorHandler.hpp"
#include "FileController.hpp"

// REVIEW : request 쪽과 동일한 검토가 필요합니다.
#define RESPONSE_BUFFER_SIZE 32768

class HTTPResponseHandler {
    private:
        typedef enum e_Phase {FILEOPEN, ASSEMBLEHEADER, OK, NOTFOUND, FIN} Phase;
        Phase _phase;
        bool _finish;
        int _sessionFd;
        std::string _fileURI;
        std::string _root;
        std::map<std::string, std::string> _headers;
        std::string _headerString;
        std::string _internalHTMLString;
        FileController file;
        HTTPResponseHandler();
    public:
        HTTPResponseHandler(int sessionFd);
        HTTPResponseHandler(int sessionFd, std::string arg);
        ~HTTPResponseHandler();
        void process(void);
        bool isFinish(void);
    private:
        void buildGeneralHeader(std::string status);
        void buildOKHeader(std::string type, long contentLength);
        void convertHeaderMapToString(void);
        static std::string& get404Body(void);
        std::string getMINE(std::string extension);
        std::string getExtenstion(std::string& URI);
};

#endif
