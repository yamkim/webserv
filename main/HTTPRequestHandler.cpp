#include "HTTPRequestHandler.hpp"

HTTPRequestHandler::HTTPRequestHandler(int connectionFd) : HTTPHandler(connectionFd) {
    // REVIEW : accept에서 클라이언트의 IP와 포트도 받을 수 있도록 하면 좋을것 같습니다.
    _phase = PARSE_STARTLINE;
    _contentLength = -1;
    _fileController = NULL;
}

HTTPRequestHandler::~HTTPRequestHandler() {
    delete _fileController;
}

HTTPRequestHandler::Phase HTTPRequestHandler::process() {
    // NOTE : 클라이언트로부터 데이터를 완전히 수신할 때까지의 동작을 제어하는 메인 메소드입니다.
    if (_phase == PARSE_STARTLINE) {
        if (getRequestLine() == true) {
            _phase = PARSE_HEADER;
        } else {
            _phase = PARSE_STARTLINE;
        }
    } else if (_phase == PARSE_HEADER) {
        // TODO : 클라이언트가 이상한 헤더를 보낼 때 적절한 처리가 필요합니다. exception을 여기서 잡거나. 아직 자세히 안봤지만 Nginx는 400 Bad Request 같은거로 핸들링하는거 같습니다. 추후에 적용하겠습니다.
        if (getHeader() == true) {
            if (_headers.find("Content-Length") == _headers.end()) {
                _phase = FINISH;
            } else {
                _contentLength = std::atoi(_headers["Content-Length"].c_str());
                std::cout << "_headers[] : " << _headers["Content-Length"] << std::endl;
                char tmp[10];
                srand(time(NULL));
                for (int j = 0; j < 10; j++) {
                    tmp[j] = char(std::rand() % ('Z' - 'A') + 'A');
                }
                _fileController = new FileController(std::string(tmp, 10), FileController::WRITE);
                _phase = PARSE_BODY;
            }
            // TODO : POST일 경우 임시 파일 만들어서 저장하는 로직 추가 예정
        } else {
            _phase = PARSE_HEADER;
        }
    } else if (_phase == PARSE_BODY) {
        std::cout << "_contentLength : " << _contentLength << std::endl;
        char buffer[REQUEST_BUFFER_SIZE + 1];

        int readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, 0);
        std::cout << "readLength : " << readLength << std::endl;
        _contentLength -= readLength;
        if (_contentLength <= 0) {
            std::cout << "readLength + _contentLength : " << readLength + _contentLength << std::endl;
            readLength = write(_fileController->getFd(), buffer, readLength + _contentLength);
            _phase = FINISH;
        } else {
            readLength = write(_fileController->getFd(), buffer, readLength);
        }
    }
    return _phase;
}

bool HTTPRequestHandler::getRequestLine(void) {
    if (setHeaderString() == false) {
        return (false);
    }

    _requestLine = _headerString;
    std::vector<std::string> tmp = Parser::getSplitBySpace(_requestLine);
    if (tmp.size() != 3) {
        throw ErrorHandler("Error: invalid _headerString.", ErrorHandler::ALERT, "HTTPRequestHandler::getRequestLine");
    } else {
        setMethod(tmp[0]);
        setURI(tmp[1]);
        setProtocol(tmp[2]);
    }
    _headerString.clear();
    return (true);
}

bool HTTPRequestHandler::getHeader(void) {
    // REVIEW: if 두개의 의미를 모르겠습니다!
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

void HTTPRequestHandler::setMethod(std::string method) {
    if (method == std::string("GET")) {
        _method = HTTPRequestHandler::GET;
    } else if (method == std::string("POST")) {
        _method = HTTPRequestHandler::POST;
    } else if (method == std::string("DELETE")) {
        _method = HTTPRequestHandler::DELETE;
    } else {
        throw ErrorHandler("Error: weird method.", ErrorHandler::ALERT, "HTTPRequestHandler::getRequestLine");
    }
}

void HTTPRequestHandler::setURI(std::string URI) {
    if (!URI.empty()) {
        _URI = URI;
    } else {
        throw ErrorHandler("Error: empty URI.", ErrorHandler::ALERT, "HTTPRequestHandler::getRequestLine");
    }
}

void HTTPRequestHandler::setProtocol(std::string protocol) {
    if (protocol != std::string("HTTP/1.1")) {
        throw ErrorHandler("Error: weird Protocol.", ErrorHandler::ALERT, "HTTPRequestHandler::process");
    }
}