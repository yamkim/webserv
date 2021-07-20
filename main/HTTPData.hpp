#ifndef HTTPDATA_HPP
#define HTTPDATA_HPP

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <map>
#include "FileController.hpp"

#include <iostream> //FIXME 디버깅용
class HTTPData {
    public:
        // Connection Data
        std::string _hostIP;
        std::string _hostPort;
        std::string _clientIP;
        std::string _clientPort;

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
        std::string _originURI;
        std::map<std::string, std::string> _HTTPCGIENV;
        
        // Response Data
        int _statusCode;
        std::string _serverName;
        std::string _root;
        std::string _CGIBinary;
        std::string _resAbsoluteFilePath;
        long _resContentLength;

        // Common Data
        std::string _postFilePath;

        HTTPData();
        ~HTTPData();

        // Request Data Method
        std::string getMethod(void) const;
        std::string getURI(void) const;
        static std::string getExtension(std::string URI);
        void setURIelements(void);
        static std::string& getResStartLineMap(int code);
        void setHTTPCGIENV(std::map<std::string, std::string> headers);
};

#endif