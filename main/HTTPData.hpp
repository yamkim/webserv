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
        std::string _URIFilePath;     // /data/index.html
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
        std::string _CGIBinary;

        // Common Data
        std::string _postFilePath;

        // Request Data Method
        std::string getMethod(void) const {
            return (_reqMethod);
        }

        std::string getURI(void) const {
            return (_reqURI);
        }

        void setURIelements(void) {
            std::size_t foundQuestion = _reqURI.rfind("?");
            if (foundQuestion == std::string::npos) {
                _URIFilePath = _reqURI;
            } else {
                _URIQueryString = _reqURI.substr(foundQuestion + 1);
                _URIFilePath = _reqURI.substr(0, foundQuestion);
            }
            std::size_t foundDot = _URIFilePath.rfind(".");
            std::size_t foundSlash = _URIFilePath.rfind("/");
            _URIFileName = _URIFilePath.substr(foundSlash + 1);
            if (foundDot != std::string::npos && foundSlash <= foundDot) {
                _URIExtension = _URIFilePath.substr(foundDot + 1);
            }
        }
};

#endif