#include "HTTPHandler.hpp"

HTTPHandler::Buffer::Buffer(size_t bufferSize) {
    _buffer = new char[bufferSize];
    if (_buffer == NULL) {
        throw ErrorHandler("Error: memory alloc error", ErrorHandler::CRITICAL, "HTTPHandler::Buffer::Buffer");
    }
}

HTTPHandler::Buffer::~Buffer() {
    delete [] _buffer;
}

char* HTTPHandler::Buffer::operator*(void) {
    return (_buffer);
}

HTTPHandler::HTTPHandler(int connectionFd, NginxConfig::ServerBlock serverConf, const NginxConfig& nginxConf) : _serverConf(serverConf), _nginxConf(nginxConf) {
	_connectionFd = connectionFd;
	_headerString = std::string("");
}

HTTPHandler::~HTTPHandler() {}

void HTTPHandler::setGeneralHeader(std::string status) {
    static char timeBuffer[48];
    time_t rawtime;

    std::time(&rawtime);
    struct tm* timeinfo = std::gmtime(&rawtime);
    std::strftime(timeBuffer, 48, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

    _headerString = status;
    _headerString += "\r\n";

    _headers["Date"] = std::string(timeBuffer);
    _headers["Server"] = std::string("webserv/0.1");
    _headers["Connection"] = std::string("close");
}

void HTTPHandler::convertHeaderMapToString(void) {
    std::map<std::string, std::string>::iterator iter;
    for (iter = _headers.begin(); iter != _headers.end(); ++iter) {
        _headerString += iter->first;
        _headerString += ": ";
        _headerString += iter->second;
        _headerString += "\r\n";
    }
    _headerString += "\r\n";
}

std::pair<std::string, std::string> HTTPHandler::getHTTPHeader(const std::string& str, std::size_t& endPos) {
    std::pair<std::string, std::string> pair;
    pair.first = Parser::getIdentifier(str, endPos, ": ");
    if (pair.first.empty()) {
        throw ErrorHandler("Error: HTTP Header error.", ErrorHandler::ALERT, "HTTPHandler::getHTTPHeader");
    }
    endPos += 2;
    pair.second = Parser::getIdentifier(str, endPos, "\r\n");
    if (pair.second.empty()) {
        throw ErrorHandler("Error: HTTP Header error.", ErrorHandler::ALERT, "HTTPHandler::getHTTPHeader");
    }
    endPos += 2;
    return (pair);
}
