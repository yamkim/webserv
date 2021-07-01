#ifndef HTTPDATA_HPP
#define HTTPDATA_HPP

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>

class HTTPData {
    public:
        // Connection Data
        std::string _hostIP;
        int _hostPort;
        std::string _clientIP;
        int _clientPort;

        // localhost:4242/data/index.html?id=123
        std::string _URIAbsolutePath; // root/data/index.html
        std::string _URIQueryString;  // id=123
        std::string _URIExtension;    // html
        std::string _URIFileName;     // index.html

        // Request Data
        std::string _reqURI;
        std::string _reqMethod;
        std::string _reqContentType;
        std::string _reqContentLength;
        std::string _requestFilePath;
        std::string _requestAbsoluteFilePath;
        
        // Response Data
        int _statusCode;

        // Common Data
        std::string _postFilePath;

        // Request Data Method
        std::string getMethod(void) const {
            return (_reqMethod);
        }

        std::string getURI(void) const {
            return (_reqURI);
        }

        void setExtension(void) {
            std::size_t foundDot = _reqURI.rfind(".");
            std::size_t foundSlash = _reqURI.rfind("/");
            if (foundDot == std::string::npos || foundSlash > foundDot) {
                _URIExtension = std::string("");
            }
            _URIExtension = _reqURI.substr(foundDot + 1);
        }
};

#endif