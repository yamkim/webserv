#include "HTTPRequestHandler.hpp"

HTTPRequestHandler::HTTPRequestHandler(int connectionFd) : HTTPHandler(connectionFd) {
    // REVIEW : accept에서 클라이언트의 IP와 포트도 받을 수 있도록 하면 좋을것 같습니다.
    _phase = PARSESTARTLINE;
}

HTTPRequestHandler::~HTTPRequestHandler() {}

void HTTPRequestHandler::process(void) {
    // NOTE : 클라이언트로부터 데이터를 완전히 수신할 때까지의 동작을 제어하는 메인 메소드입니다.
    char temp;

    if (recv(_connectionFd, &temp, 1, MSG_PEEK) == 0) {
        // NOTE : 브라우저가 처음 접속할 때 핸드쉐이킹만 할 때 혹은 req 도중 갑자기 연결이 끊어질 때에 대한 핸들링입니다.
        _phase = CONNECTIONCLOSE;
        return ;
    }
    if (_phase == PARSESTARTLINE) {
        if (getHeaderStartLine() == true) {
            std::cout << "[DEBUG] HTTPRequestHandler.URI : " << _URI << std::endl;
            _phase = PARSEHEADER;
        }
    } else if (_phase == PARSEHEADER) {
        // TODO : 클라이언트가 이상한 헤더를 보낼 때 적절한 처리가 필요합니다. exception을 여기서 잡거나. 아직 자세히 안봤지만 Nginx는 400 Bad Request 같은거로 핸들링하는거 같습니다. 추후에 적용하겠습니다.
        if (getHeader() == true) {
            _phase = FINISH;
            // TODO : POST일 경우 임시 파일 만들어서 저장하는 로직 추가 예정
        }
    }
}

bool HTTPRequestHandler::isFinish(void) {
    return ((_phase == CONNECTIONCLOSE) || (_phase == FINISH));
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

bool HTTPRequestHandler::readBufferTillNewLine(void) {
    char buffer[REQUEST_BUFFER_SIZE + 1];

    int readLength = recv(_connectionFd, buffer, REQUEST_BUFFER_SIZE, MSG_PEEK);
    if (readLength == TRANS_ERROR) {
        throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPRequestHandler::readBufferTillNewLine");
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

    if (readBufferTillNewLine() == false) {
        return (false);
    }
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
    if (chunk.size() != 0) {
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

    if (readBufferTillNewLine() == false) {
        return (false);
    }
    if (_headerString.length() < 3) {
        _headerString.clear();
        return (true);
    }
    key = getStringHeadByDelimiter(_headerString, delimiterLength, ": ");
    if (key.length() == 0) {
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

bool HTTPRequestHandler::isConnectionCloseByClient(void) {
    return ((_phase == CONNECTIONCLOSE));
}
