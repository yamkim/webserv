#include "HTTPRequestHandler.hpp"

HTTPRequestHandler::HTTPRequestHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, NginxConfig::NginxConfig& nginxConf) : HTTPHandler(connectionFd, serverConf, nginxConf) {
    _phase = PARSE_STARTLINE;
    _contentLength = -1;
    _fileController = NULL;
}

HTTPRequestHandler::~HTTPRequestHandler() {
    delete _fileController;
}

HTTPRequestHandler::Phase HTTPRequestHandler::process(HTTPData& data) {
    if (_phase == PARSE_STARTLINE) {
        data._statusCode = 200;
        if (getStartLine(data) == true) {
            _phase = PARSE_HEADER;
        }
    } else if (_phase == PARSE_HEADER) {
        if (getHeader() == true) {
            if (_headers.find("Content-Length") == _headers.end()) {
                _phase = FINISH;
            } else {
                _contentLength = std::atoi(_headers["Content-Length"].c_str());
                if (_contentLength > std::atoi(_serverConf.dirMap["client_max_body_size"].c_str())) {
                    data._statusCode = 413;
                    _phase = FINISH;
                } else {
                    char tmp[10];
                    srand(time(NULL));
                    for (int j = 0; j < 10; j++) {
                        tmp[j] = char(std::rand() % ('Z' - 'A') + 'A');
                    }
                    data._postFilePath = std::string(tmp, 10);
                    _fileController = new FileController(data._postFilePath, FileController::WRITE);
                    data._reqContentType = _headers["Content-Type"];
                    data._reqContentLength = _headers["Content-Length"];
                    _phase = PARSE_BODY;
                }
            }
        } else {
            _phase = PARSE_HEADER;
        }
    } else if (_phase == PARSE_BODY) {
        char buffer[REQUEST_BUFFER_SIZE + 1];
        int readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, 0);
        if (readLength == -1) {
            throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::process");
        }
        _contentLength -= readLength;
        if (_contentLength <= 0) {
            readLength = write(_fileController->getFd(), buffer, readLength + _contentLength);
            _phase = FINISH;
        } else {
            readLength = write(_fileController->getFd(), buffer, readLength);
        }
        if (readLength == -1) {
            throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::process");
        }
    }
    return (_phase);
}

bool HTTPRequestHandler::getStartLine(HTTPData& data) {
    if (setHeaderString() == false) {
        return (false);
    }
    _requestLine = _headerString;
    std::vector<std::string> tmp = Parser::getSplitBySpace(_requestLine);
    if (tmp.size() != 3) {
        throw ErrorHandler("Error: invalid HTTP Header.", ErrorHandler::NORMAL, "HTTPRequestHandler::getStartLine");
    }
    if (   tmp[0] == std::string("GET")
        || tmp[0] == std::string("HEAD")
        || tmp[0] == std::string("POST")
        || tmp[0] == std::string("PUT")
        || tmp[0] == std::string("DELETE")
        || tmp[2] != std::string("HTTP/1.1")) {
        data._reqMethod = tmp[0];
        data._reqURI = tmp[1];
        data._originURI = tmp[1];
        data.setURIelements();
    } else {
        throw ErrorHandler("Error: invalid HTTP Header.", ErrorHandler::NORMAL, "HTTPRequestHandler::process");
    }
    _headerString.clear();
    return (true);
}

bool HTTPRequestHandler::getHeader(void) {
    if (setHeaderString() == false) {
        return (false);
    }
    if (_headerString == std::string("\r\n")) {
        _headerString.clear();
        return (true);
    }
    std::size_t pos = 0;
    _headers.insert(getHTTPHeader(_headerString, pos));
    _headerString.clear();
    return (false);
}

int HTTPRequestHandler::findNewLine(const char *buffer) {
    const char* n = std::strstr(buffer, "\r\n");
    if (n == NULL) {
        return (-1);
    } else {
        return (n - buffer + 2);
    }
}

bool HTTPRequestHandler::setHeaderString(void) {
    char buffer[REQUEST_BUFFER_SIZE + 1];
    ssize_t readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, MSG_PEEK);
    if (readLength == TRANS_ERROR) {
        throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::setHeaderString");
    }
    buffer[readLength] = '\0';
    int newLinePosition = findNewLine(buffer);
    if (newLinePosition == -1) {
        readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, 0);
        if (readLength == TRANS_ERROR) {
            throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::setHeaderString");
        }
        _headerString += std::string(buffer, readLength);
        return (false);
    } else {
        readLength = recv(_connectionFd, buffer, newLinePosition, 0);
        if (readLength == TRANS_ERROR) {
            throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::setHeaderString");
        }
        _headerString += std::string(buffer, readLength);
        return (true);
    }
}
