#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include <string>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include "ErrorHandler.hpp"
#include "HTTPData.hpp"
#include "Utils.hpp"
#include "NginxConfig.hpp"

#ifndef WEBSERV_VERSION
#define WEBSERV_VERSION "0.0.0"
#endif

#define REQUEST_BUFFER_SIZE 1000

class HTTPHandler {
    private:
        HTTPHandler();

    protected:
        static const ssize_t TRANS_ERROR = -1;
        std::map<std::string, std::string> _headers;
        int _connectionFd;
        std::string _headerString;
        NginxConfig::ServerBlock _serverConf;
        NginxConfig::NginxConfig _nginxConf;
        class Buffer {
            private:
                int _bufferSize;
                char* _buffer;
                Buffer();
            public:
                Buffer(size_t bufferSize);
                ~Buffer();
                char* operator*(void);
        };

    public:
        HTTPHandler(int connectionFd, NginxConfig::ServerBlock serverConf, const NginxConfig::NginxConfig& nginxConf);
        virtual ~HTTPHandler();

        void setGeneralHeader(std::string status);
        void convertHeaderMapToString(void);
        std::pair<std::string, std::string> getHTTPHeader(const std::string& str, std::size_t& endPos);
};
#endif
