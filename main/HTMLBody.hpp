#ifndef HTMLBODY_HPP
#define HTMLBODY_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include "HTTPData.hpp"
#include "FileController.hpp"
#include "ErrorHandler.hpp"

#ifndef WEBSERV_VERSION
#define WEBSERV_VERSION "0.0.0"
#endif

class HTMLBody {
    public:
        static std::string getBasicHTMLBody(const std::string& statusMsg);
        static std::string getAutoIndexBody(std::string root, std::string path);
        static std::string getRedirectBody(const HTTPData& data);
        static std::string getStaticHTML(const HTTPData& data);
};
#endif