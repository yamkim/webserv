#include "HTTPRequestHandler.hpp"

HTTPRequestHandler::HTTPRequestHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, NginxConfig& nginxConf) : HTTPHandler(connectionFd, serverConf, nginxConf) {
    // REVIEW : accept에서 클라이언트의 IP와 포트도 받을 수 있도록 하면 좋을것 같습니다.
    _phase = PARSE_STARTLINE;
    _contentLength = -1;
    _fileController = NULL;
}

HTTPRequestHandler::~HTTPRequestHandler() {
    delete _fileController;
}

HTTPRequestHandler::Phase HTTPRequestHandler::process(HTTPData& data) {
    // NOTE : 클라이언트로부터 데이터를 완전히 수신할 때까지의 동작을 제어하는 메인 메소드입니다.
    if (_phase == PARSE_STARTLINE) {
        data._statusCode = 200;
        if (getRequestLine(data) == true) {
            _phase = PARSE_HEADER;
        } else {
            _phase = PARSE_STARTLINE;
        }
    } else if (_phase == PARSE_HEADER) {
        if (getHeader() == true) {
            if (_headers.find("Content-Length") == _headers.end()) {
                _phase = FINISH;
            } else {
                _contentLength = std::atoi(_headers["Content-Length"].c_str());
                // std::cout << "_headers[] : " << _headers["Content-Length"] << std::endl;
                char tmp[10];
                srand(time(NULL));
                for (int j = 0; j < 10; j++) {
                    tmp[j] = char(std::rand() % ('Z' - 'A') + 'A');
                }
                data._postFilePath = std::string(tmp, 10);
                _fileController = new FileController(std::string(tmp, 10), FileController::WRITE);
                data._reqContentType = _headers["Content-Type"];
                data._reqContentLength = _headers["Content-Length"];
                _phase = PARSE_BODY;
            }
        } else {
            _phase = PARSE_HEADER;
        }
    } else if (_phase == PARSE_BODY) {
        // std::cout << "_contentLength : " << _contentLength << std::endl;
        char buffer[REQUEST_BUFFER_SIZE + 1];

        int readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, 0);
        // std::cout << "readLength : " << readLength << std::endl;
        _contentLength -= readLength;
        if (_contentLength <= 0) {
            // std::cout << "readLength + _contentLength : " << readLength + _contentLength << std::endl;
            readLength = write(_fileController->getFd(), buffer, readLength + _contentLength);
            _phase = FINISH;
        } else {
            readLength = write(_fileController->getFd(), buffer, readLength);
        }
    }
    return _phase;
}

bool HTTPRequestHandler::getRequestLine(HTTPData& data) {
    if (setHeaderString() == false) {
        return (false);
    }

    _requestLine = _headerString;
    std::vector<std::string> tmp = Parser::getSplitBySpace(_requestLine);
    if (tmp.size() != 3) {
        throw ErrorHandler("Error: invalid request line.", ErrorHandler::ALERT, "HTTPRequestHandler::getRequestLine");
    }
    if (   tmp[0] == std::string("GET")
        || tmp[0] == std::string("POST")
        || tmp[0] == std::string("DELETE")
        || tmp[2] != std::string("HTTP/1.1")) {
        data._reqMethod = tmp[0];
        data._reqURI = tmp[1];
        data.setURIelements();
    } else {
        throw ErrorHandler("Error: invalid request line.", ErrorHandler::ALERT, "HTTPRequestHandler::process");
    }
    _headerString.clear();

    return (true);
}

bool HTTPRequestHandler::getHeader(void) {
    if (setHeaderString() == false) {
        return (false);
    }
    if (_headerString.length() < 3) {
        _headerString.clear();
        return (true);
    }

    std::size_t pos = 0;
    std::string key = Parser::getIdentifier(_headerString, pos, ": ");
    if (key.empty()) {
        throw ErrorHandler("Error: HTTP Header error.", ErrorHandler::ALERT, "HTTPRequestHandler::getHeader");
    }
    pos += 2;
    std::string value = Parser::getIdentifier(_headerString, pos, "\r\n");
    if (value.empty()) {
        throw ErrorHandler("Error: HTTP Header error.", ErrorHandler::ALERT, "HTTPRequestHandler::getHeader");
    }
    _headers[key] = value;
    _headerString.clear();
    return (false);
}

int HTTPRequestHandler::findNewLine(const char *buffer) {
    const char* n = std::strstr(buffer, "\n");
    if (n == NULL) {
        return (-1);
    } else {
        return (n - buffer + 1);
    }
}

bool HTTPRequestHandler::setHeaderString(void) {
    char buffer[REQUEST_BUFFER_SIZE + 1];

    int readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, MSG_PEEK);
    if (readLength == TRANS_ERROR) {
        throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::setHeaderString");
    }
    buffer[readLength] = '\0';
    int newLinePosition = findNewLine(buffer);
    if (newLinePosition == -1) {
        readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, 0);
        _headerString += std::string(buffer, readLength);
        return (false);
    } else {
        readLength = recv(_connectionFd, buffer, newLinePosition, 0);
        _headerString += std::string(buffer, readLength);
        return (true);
    }
}
