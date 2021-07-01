#include "HTTPResponseHandler.hpp"

// FIXME : 리스폰스에 URI 외에 더 다양한 아규먼트를 집어넣어야 하는데 어떤 형식으로 집어넣을지 고민 중입니다. 추후에 수정하겠습니다.
HTTPResponseHandler::HTTPResponseHandler(int connectionFd, const NginxConfig& nginxConf) : HTTPHandler(connectionFd, nginxConf) {
    _phase = FIND_RESOURCE;
    // FIXME : root 경로와 같은 정보는 .conf 파일에서 받아와야 합니다.
    _root = _nginxConf._http.server[1].dirMap["root"];
    _file = NULL;

    // FIXME: 일단 공통 구조체를 process 내에서만 사용해서... 이 변수들에 대해 조치가 필요합니다.
    _serverIndex = getServerIndex(_nginxConf._http.server[1]);

    _cgi = NULL;
}

HTTPResponseHandler::~HTTPResponseHandler() {
    delete _cgi;
    delete _file;
}

std::string HTTPResponseHandler::getServerIndex(NginxConfig::ServerBlock server) {
    for (int i = 0; i < (int)server.index.size(); i++) {
        if (FileController::checkType(_absolutePath + server.index[i]) == FileController::FILE) {
            return (server.index[i]);
        }
    }
    return (std::string(""));
}

void HTTPResponseHandler::responseNotFound(const HTTPData& data) {
    std::string notFoundType = std::string("html");
    _staticHtml = HTMLBody::getStaticHTML(data._statusCode);
    setHTMLHeader("html", _staticHtml.length());
    send(_connectionFd, _headerString.data(), _headerString.length(), 0);
    _phase = DATA_SEND_LOOP;
}

void HTTPResponseHandler::responseAutoIndex(const HTTPData& data) {
    std::string autoIndexType = std::string("html");
    _staticHtml = HTMLBody::getAutoIndexBody(_root, data._URIFilePath);
    setHTMLHeader("html", _staticHtml.length());
    send(_connectionFd, _headerString.data(), _headerString.length(), 0);
    _phase = DATA_SEND_LOOP;
}

void HTTPResponseHandler::setHTMLHeader(const std::string& extension, const long contentLength) {
    setTypeHeader(getMIME(extension));
    setLengthHeader(contentLength);
    convertHeaderMapToString(false);
}

HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data) {
    if (_phase == FIND_RESOURCE) {
        // TODO: data 인수에 request 파싱된 결과가 들어있어서 이 클래스 초기화될때 data를 넣어서 초기화 하거나 여기서 초기화 해야합니다.
        _absolutePath = _root + data._URIFilePath;
        _type = FileController::checkType(_absolutePath);
        data._requestAbsoluteFilePath = _absolutePath + _serverIndex;
        if (_type == FileController::NOTFOUND) {
            setGeneralHeader("HTTP/1.1 404 Not Found");
            _phase = NOT_FOUND;
        } else if (_type == FileController::DIRECTORY) {
            setGeneralHeader("HTTP/1.1 200 OK");
            if (_serverIndex.empty()) { // 만약 _serverIndex가 없다면 바로 auto index로 띄우기
                _phase = AUTOINDEX;
            } else {                    // 만약 dierctory이고 server index가 있다면 절대 경로에 index 더하기
                _absolutePath = _absolutePath + _serverIndex;
                _phase = GET_FILE;
            }
        } else if (_type == FileController::FILE) {
            setGeneralHeader("HTTP/1.1 200 OK");
            if (isCGI(data._URIExtension)) {
                _phase = CGI_RUN;
            } else {
                _phase = GET_FILE;
            }
        }
    } 

    if (_phase == NOT_FOUND) {
        responseNotFound(data);
    }

    if (_phase == AUTOINDEX) {
        responseAutoIndex(data);
    }
    
    if (_phase == GET_FILE) {
        _file = new FileController(_absolutePath, FileController::READ);
        setHTMLHeader(data._URIExtension, _file->length());
        send(_connectionFd, _headerString.data(), _headerString.length(), 0);
        _phase = DATA_SEND_LOOP;
    }
    
    #if 1 // HTML Part
    if (_phase == DATA_SEND_LOOP) {
        // FIXME: 조금 더 이쁘장하게 수정해야 할 듯 합니다...
        if (_staticHtml.empty() == false) {
            size_t writeLength = send(_connectionFd, _staticHtml.c_str(), _staticHtml.length(), 0);
            if (writeLength != _staticHtml.length()) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            } else {
                _phase = FINISH;
            }
        } else {
            char buf[RESPONSE_BUFFER_SIZE];
            int length = read(_file->getFd(), buf, RESPONSE_BUFFER_SIZE);
            if (length != 0) {
                int writeLength = send(_connectionFd, buf, length, 0);
                if (writeLength != length) {
                    throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
                }
            } else {
                _phase = FINISH;
            }
        }
    } 
    #endif

    #if 1 // CGI Part
    if (_phase == CGI_RUN) {
        // FIXME[yekim]: _cgi를 처음에 절대 경로와 함께 생성하는데, 이를 그대로 사용하면 cgi가 동작하지 않습니다 ㅠㅠ
        delete _cgi;
        if (data._URIExtension == std::string("py")) {
            data._CGIBinary = std::string("/usr/bin/python3");
        } else if (data._URIExtension == std::string("php")) {
            data._CGIBinary = std::string("/Users/joohongpark/Desktop/webserv_workspace/webserv/main/php-bin/php-cgi");
        }
        _cgi = new CGISession(data);
        _cgi->makeCGIProcess();
        fcntl(_cgi->getOutputStream(), F_SETFL, O_NONBLOCK);
        convertHeaderMapToString(true);
        send(_connectionFd, _headerString.data(), _headerString.length(), 0);
        _phase = CGI_REQ;
    }
    
    if (_phase == CGI_REQ) {
        if (data._postFilePath.empty()) {
            _phase = CGI_RECV_LOOP;
        } else {
            _file = new FileController(data._postFilePath, FileController::READ);
            _phase = CGI_SEND_LOOP;
        }
    }
    
    if (_phase == CGI_SEND_LOOP) {
        char buf[RESPONSE_BUFFER_SIZE];

        int length = read(_file->getFd(), buf, RESPONSE_BUFFER_SIZE);
        if (length != 0) {
            int writeLength = write(_cgi->getInputStream(), buf, length);
            if (writeLength != length) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            }
        } else {
            _file->del();
            _phase = CGI_RECV_LOOP;
        }
    }
    
    if (_phase == CGI_RECV_LOOP) {
        char buf[RESPONSE_BUFFER_SIZE];

        int length = read(_cgi->getOutputStream(), buf, RESPONSE_BUFFER_SIZE);
        if (length != 0) {
            int writeLength = send(_connectionFd, buf, length, 0);
            if (writeLength != length) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            }
        } else {
            _phase = FINISH;
        }
    }
    #endif
    return (_phase);
}

bool HTTPResponseHandler::isCGI(std::string& URI) {
    // FIXME: nginx.conf 파일 파싱 결과를 토대로 이 확장명이 CGI인지 판단해야 합니다.
    if (URI == std::string("py") || URI == std::string("php")) {
        return (true);
    } else {
        return (false);
    }
}

int HTTPResponseHandler::getCGIfd(void) {
    return (_cgi->getOutputStream());
}

std::string HTTPResponseHandler::getMIME(const std::string& extension) const {
    // FIXME : yekim : types 블록에 대해 map 타입으로 파싱해야 합니다.

    std::map<std::string, std::string> mime = _nginxConf._http.types.typeMap;
    if(extension == std::string("") || mime.find(extension) == mime.end()) {
        return (std::string("application/octet-stream"));
    } else {
        return (mime[extension]);
    }
}