#ifndef HTTPRESPONSEHANDLER_HPP
#define HTTPRESPONSEHANDLER_HPP

#include <ctime>
#include <sstream>
#include <iostream>

#include "FileController.hpp"
#include "HTTPHandler.hpp"

class HTTPResponseHandler : public HTTPHandler {
    private:
        typedef enum e_Phase {FILEOPEN, ASSEMBLEHEADER, OK, NOTFOUND, FINISH} Phase;
        Phase _phase;
        std::string _root;
        std::string _internalHTMLString;
        FileController file;
        HTTPResponseHandler();
    public:
        HTTPResponseHandler(int sessionFd);
        HTTPResponseHandler(int sessionFd, std::string arg);
        virtual ~HTTPResponseHandler();
        virtual void process(void);
        virtual bool isFinish(void);
    private:
        void buildGeneralHeader(std::string status);
        void buildOKHeader(std::string type, long contentLength);
        void convertHeaderMapToString(void);
        static std::string& get404Body(void);
        std::string getMIME(std::string extension);
        std::string getExtenstion(std::string& URI);
};

#endif
