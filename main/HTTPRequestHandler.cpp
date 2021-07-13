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
            std::cout << "[DEBUG] STEP 0 =========================" << std::endl; 
            _phase = PARSE_HEADER;
        }
    } else if (_phase == PARSE_HEADER) {
        // std::cout << "[DEBUG] STEP 1 =========================" << std::endl; 
        if (getHeader() == true) {
            // std::cout << "[DEBUG] STEP 2 =========================" << std::endl; 
            if (_headers.find("Content-Length") == _headers.end()) {
                // std::cout << "[DEBUG] STEP 3 =========================" << std::endl; 
                _phase = FINISH;
            } else {
                // std::cout << "[DEBUG] STEP 4 =========================" << std::endl; 
                _contentLength = std::atoi(_headers["Content-Length"].c_str());
                if (_contentLength > std::atoi(_serverConf.dirMap["client_max_body_size"].c_str())) {
                    // std::cout << "[DEBUG] STEP 5 =========================" << std::endl; 
                    data._statusCode = 413;
                    _phase = FINISH;
                } else {
                    // std::cout << "[DEBUG] STEP 6 =========================" << std::endl; 
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
        || tmp[0] == std::string("POST")
        || tmp[0] == std::string("PUT")
        || tmp[0] == std::string("DELETE")
        || tmp[2] != std::string("HTTP/1.1")) {
        data._reqMethod = tmp[0];
        data._reqURI = tmp[1];
        data.setURIelements();
    } else {
        throw ErrorHandler("Error: invalid HTTP Header.", ErrorHandler::NORMAL, "HTTPRequestHandler::process");
    }
    _headerString.clear();
    return (true);
}

void HTTPRequestHandler::showHeader(void) {
    std::map<std::string, std::string>::iterator iter = _headers.begin();
    std::cout << "[DEBUG] SHOW HEADER =====================================" << std::endl;
    for (; iter != _headers.end(); ++iter) {
        std::cout << "[DEBUG] _headers[" << iter->first << "]: [" << iter->second << "]" << std::endl;
    }
}

bool HTTPRequestHandler::getHeader(void) {
    bool tmpFlag = setHeaderString();
    if (tmpFlag == false) {
        return (false);
    }
    std::cout << "[DEBUG] _headerString before checking: [" << _headerString << "]" << std::endl;
    if (_headerString == std::string("\r\n")) {
        std::cout << "[DEBUG] LINELINELINE================================ " << std::endl;
        _headerString.clear();
        return (true);
    }
    std::size_t pos = 0;
    _headers.insert(getHTTPHeader(_headerString, pos));
    
    showHeader();
    _headerString.clear();
    return (false);
}

#if 0
int HTTPRequestHandler::findNewLine(const char *buffer) {
    const char* n = std::strstr(buffer, "\r\n");
    if (n == NULL) {
        return (-1);
    } else {
        return (n - buffer + 2);
    }
}
#endif

int HTTPRequestHandler::findNewLine(const char *buffer) {
    const char* n = std::strstr(buffer, "\r\n");
    if (n == NULL) {
        bool case2 = (buffer[0] == '\n' && (!_headerString.empty() && _headerString[_headerString.length()-1] == '\r'));
        if (case2) {
            return (1);
        }
        else
            std::cout << "개행 없음" << std::endl;
        return (-1);
    } else {
        return (n - buffer + 2);
    }
}

bool HTTPRequestHandler::setHeaderString(void) {
    char buffer[REQUEST_BUFFER_SIZE + 1];
    ssize_t readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, MSG_PEEK);
    // std::cout << "[DEBUG] BUFFER in setHeaderString: [" << buffer << "]" << std::endl;
    if (readLength == TRANS_ERROR) {
        throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::setHeaderString");
    }
    buffer[readLength] = '\0';

    int newLinePosition = findNewLine(buffer);
    if (newLinePosition == -1) {
        readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, 0);
        std::cout << "[DEBUG] BUFFER after recv: [" << buffer << "]" << std::endl;
        if (readLength == TRANS_ERROR) {
            throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::setHeaderString");
        }
        _headerString += std::string(buffer, readLength);
        return (false);
    } else {
        readLength = recv(_connectionFd, buffer, newLinePosition, 0);
        std::cout << "[DEBUG] buffer with newLine: [" << buffer << "]" << std::endl;
        if (readLength == TRANS_ERROR) {
            throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::setHeaderString");
        }
        std::cout << "[DEBUG] ================================" << std::endl;
        _headerString += std::string(buffer, readLength);
        return (true);
    }
}