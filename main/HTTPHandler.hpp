#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include <string>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include "ErrorHandler.hpp"
#include "HTTPData.hpp"
#include "NginxConfig.hpp"

#define REQUEST_BUFFER_SIZE 100
#define RESPONSE_BUFFER_SIZE 1000

class HTTPHandler {
    private:
        HTTPHandler();

    protected:
        static const ssize_t TRANS_ERROR = -1;
        std::map<std::string, std::string> _headers;
        int _connectionFd;
        std::string _headerString;
        NginxConfig _nginxConf;

    public:
        HTTPHandler(int connectionFd, const NginxConfig& nginxConf);
        virtual ~HTTPHandler();
        void setGeneralHeader(std::string status);
        void setTypeHeader(std::string type);
        void setLengthHeader(long contentLength);
        void convertHeaderMapToString(bool isCGI);
    
        std::map<std::string, std::string> getHeaders(void) const;
};
#endif
