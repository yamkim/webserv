#include "HTTPHandler.hpp"

HTTPHandler::HTTPHandler(int connectionFd, NginxConfig::ServerBlock serverConf, const NginxConfig& nginxConf) : _serverConf(serverConf), _nginxConf(nginxConf) {
	_connectionFd = connectionFd;
	_headerString = std::string("");
}

HTTPHandler::~HTTPHandler() {}

void HTTPHandler::convertHeaderMapToString(bool isCGI) {
    std::map<std::string, std::string>::iterator iter;
    for (iter = _headers.begin(); iter != _headers.end(); ++iter) {
        _headerString += iter->first;
        _headerString += ": ";
        _headerString += iter->second;
        _headerString += "\r\n";
    }
    if (isCGI == false) {
        _headerString += "\r\n";
    }
}