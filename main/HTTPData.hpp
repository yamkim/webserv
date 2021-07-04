#ifndef HTTPDATA_HPP
#define HTTPDATA_HPP

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream> //FIXME 디버깅용
class HTTPData {
    public:
        // Connection Data
        std::string _hostIP;
        int _hostPort;
        std::string _clientIP;
        int _clientPort;

        // localhost:4242/data/index.html?id=123
        std::string _reqURI;          // /data/index.html?id=123
        std::string _URIFilePath;     // /data/index.html
        std::string _URILocPath;      // /data/
        std::string _URIQueryString;  // id=123
        std::string _URIExtension;    // html
        std::string _URIFileName;     // index.html

        // Request Data
        std::string _reqMethod;
        std::string _reqContentType;
        std::string _reqContentLength;
        
        // Response Data
        int _statusCode;
        std::string _root;
        std::string _CGIBinary;
        std::string _resAbsoluteFilePath;

        // Common Data
        std::string _postFilePath;

        // Request Data Method
        std::string getMethod(void) const {
            return (_reqMethod);
        }

        std::string getURI(void) const {
            return (_reqURI);
        }
        
        static std::string getExtension(std::string URI) {
            std::string ret;
            std::size_t foundDot = URI.rfind(".");
            std::size_t foundSlash = URI.rfind("/");
            if (foundDot == std::string::npos 
                || (foundSlash != std::string::npos && foundSlash > foundDot)) {
                return (std::string(""));
            }
            ret = URI.substr(foundDot + 1);
            if (ret[ret.size() - 1] == '$')
                ret = ret.substr(0, ret.size() - 1);
            return ret;
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