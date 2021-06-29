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
        if (getHeaderStartLine() == true) {
            std::cout << "[DEBUG] HTTPRequestHandler.URI : " << _URI << std::endl;
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
     // TODO: yekim : 리퀘스트 바디 받아서 임시 파일에 저장하기
    // 1. 적당히 랜덤한 이름으로 tmp/[랜덤].tmp 파일로 저장
    // 2. 저장이 끝나면 임시 파일명을 리스폰스 생성자로 넘기기
    // - content_length가 없으면 종료, 있으면 length만큼 읽어오고 종료
    return _phase;
}

HTTPRequestHandler::Method HTTPRequestHandler::getMethod(void) const {
    return (_method);
}

const std::string& HTTPRequestHandler::getURI(void) const {
    return (_URI);
}
const std::map<std::string, std::string>& HTTPRequestHandler::getHeaders(void) const {
    return (_headers);
}

std::string HTTPRequestHandler::getStringHeadByDelimiter(const std::string &buf, std::size_t &pos, const std::string &needle) {
    std::string strHead;
    
    std::size_t found = buf.find(needle, pos);
    if (found != std::string::npos) {
        strHead = buf.substr(pos, found - pos);
        pos = found + needle.size();
    }
    return (strHead);
}

bool HTTPRequestHandler::setHeaderString(void) {
    char buffer[REQUEST_BUFFER_SIZE + 1];

    int readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, MSG_PEEK);
    if (readLength == TRANS_ERROR) {
        throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::setHeaderString");
    }
    buffer[readLength] = '\0';
    int newLinePosition = findNewLine(buffer);
    if (newLinePosition < 0) {
        readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, 0);
        _headerString += std::string(buffer, readLength);
        return (false);
    } else {
        readLength = recv(_connectionFd, buffer, newLinePosition, 0);
        _headerString += std::string(buffer, readLength);
        return (true);
    }
}

// TODO: Parser 클래스 만든 후 이관
int HTTPRequestHandler::findNewLine(const char *buffer) {
    const char* n = std::strstr(buffer, "\n");
    if (n == NULL) {
        return (-1);
    } else {
        return (n - buffer + 1);
    }
}

bool HTTPRequestHandler::getHeaderStartLine(void) {
    std::size_t delimiterLength = 0;
    std::string chunk;

    if (setHeaderString() == false) {
        return (false);
    }

    // TODO: 각 단계를 함수 단위로 뽑기
    chunk = getStringHeadByDelimiter(_headerString, delimiterLength, " ");
    if (chunk == std::string("GET")) {
        _method = HTTPRequestHandler::GET;
    } else if (chunk == std::string("POST")) {
        _method = HTTPRequestHandler::POST;
    } else if (chunk == std::string("DELETE")) {
        _method = HTTPRequestHandler::DELETE;
    } else {
        throw ErrorHandler("Error: weird method.", ErrorHandler::ALERT, "HTTPRequestHandler::getHeaderStartLine");
    }

    chunk = getStringHeadByDelimiter(_headerString, delimiterLength, " ");
    if (!chunk.empty()) {
        _URI = chunk;
    } else {
        throw ErrorHandler("Error: empty URI.", ErrorHandler::ALERT, "HTTPRequestHandler::getHeaderStartLine");
    }


    if (_headerString.find("\r\n") == std::string::npos) {
        chunk = getStringHeadByDelimiter(_headerString, delimiterLength, "\n");
    } else {
        chunk = getStringHeadByDelimiter(_headerString, delimiterLength, "\r\n");
    }
    if (chunk != std::string("HTTP/1.1")) {
        throw ErrorHandler("Error: weird Protocol.", ErrorHandler::ALERT, "HTTPRequestHandler::process");
    }
    _headerString.clear();
    return (true);
}

bool HTTPRequestHandler::getHeader(void) {
    std::size_t delimiterLength = 0;
    std::string key;
    std::string value;

    if (setHeaderString() == false) {
        return (false);
    }
    if (_headerString.length() < 3) {
        _headerString.clear();
        return (true);
    }
    key = getStringHeadByDelimiter(_headerString, delimiterLength, ": ");
    if (key.empty()) {
        throw ErrorHandler("Error: HTTP Header error.", ErrorHandler::ALERT, "HTTPRequestHandler::getHeader");
    }
    if (_headerString.find("\r\n") == std::string::npos) {
        value = getStringHeadByDelimiter(_headerString, delimiterLength, "\n");
    } else {
        value = getStringHeadByDelimiter(_headerString, delimiterLength, "\r\n");
    }
    _headers[key] = value;
    _headerString.clear();
    return (false);
}