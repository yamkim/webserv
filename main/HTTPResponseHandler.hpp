#ifndef HTTPRESPONSEHANDLER_HPP
#define HTTPRESPONSEHANDLER_HPP

#include <ctime>
#include <sstream>
#include <iostream>

#include "FileController.hpp"
#include "HTTPHandler.hpp"

class HTTPResponseHandler : public HTTPHandler {
    private:
        HTTPResponseHandler();
    public:
        HTTPResponseHandler(int coneectionFd, std::string arg);
        virtual ~HTTPResponseHandler();

        typedef enum e_Phase {RESOURCE_OPEN, ASSEMBLE_HEADER, OK, NOT_FOUND, FINISH} Phase;
        virtual HTTPResponseHandler::Phase process(void);
    private:
        void buildGeneralHeader(std::string status);
        void buildOKHeader(std::string type, long contentLength);
        void convertHeaderMapToString(void);
        static std::string& get404Body(void);
        std::string getMIME(std::string extension);
        std::string getExtenstion(std::string& URI);
    private:
        Phase _phase;
        std::string _root;
        std::string _internalHTMLString;
        FileController file;
};

#endif
