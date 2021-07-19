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

class HTTPHandler {
    private:
        HTTPHandler();

    protected:
        static const ssize_t TRANS_ERROR = -1;
        std::map<std::string, std::string> _headers;
        int _connectionFd;
        std::string _headerString; // FIXME: deprecated in req handler
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
                char* getBuffer() const {
                    return _buffer;
                }
        };

    public:
        HTTPHandler(int connectionFd, NginxConfig::ServerBlock serverConf, const NginxConfig::NginxConfig& nginxConf);
        virtual ~HTTPHandler();

        void setGeneralHeader(std::string status);
        void convertHeaderMapToString(void);
        std::pair<std::string, std::string> getHTTPHeader(const std::string& str, std::size_t& endPos);
        void requestAlert(std::string ip, std::string port, std::string path, std::string method);
};
#endif
