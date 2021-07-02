#include "HTTPResponseHandler.hpp"

// FIXME : 리스폰스에 URI 외에 더 다양한 아규먼트를 집어넣어야 하는데 어떤 형식으로 집어넣을지 고민 중입니다. 추후에 수정하겠습니다.
HTTPResponseHandler::HTTPResponseHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, const NginxConfig& nginxConf) : HTTPHandler(connectionFd, serverConf, nginxConf) {
    _phase = FIND_RESOURCE;
    // FIXME : root 경로와 같은 정보는 .conf 파일에서 받아와야 합니다.
    _file = NULL;

    // FIXME: 일단 공통 구조체를 process 내에서만 사용해서... 이 변수들에 대해 조치가 필요합니다.

    _cgi = NULL;
}

HTTPResponseHandler::~HTTPResponseHandler() {
    delete _cgi;
    delete _file;
}

std::string HTTPResponseHandler::getIndexFile(const std::string& absolutePath, std::vector<std::string>& indexVec) {
    std::vector<std::string>::iterator iter;
    for (iter = indexVec.begin(); iter != indexVec.end(); ++iter) {
        if (FileController::checkType(absolutePath + *iter) == FileController::FILE) {
            return (*iter);
        }
    }
    return (std::string(""));
}

void HTTPResponseHandler::responseNotFound(const HTTPData& data) {
    _staticHtml = HTMLBody::getStaticHTML(data._statusCode);
    setHTMLHeader("html", _staticHtml.length());
    send(_connectionFd, _headerString.data(), _headerString.length(), 0);
    _phase = DATA_SEND_LOOP;
}

void HTTPResponseHandler::responseAutoIndex(const HTTPData& data) {
    _staticHtml = HTMLBody::getAutoIndexBody(_root, data._URIFilePath);
    setHTMLHeader("html", _staticHtml.length());
    send(_connectionFd, _headerString.data(), _headerString.length(), 0);
    _phase = DATA_SEND_LOOP;
}

void HTTPResponseHandler::responseTest(const HTTPData& data) {
    _staticHtml = HTMLBody::getStaticHTML(data._statusCode);
    setHTMLHeader("html", _staticHtml.length());
    send(_connectionFd, _headerString.data(), _headerString.length(), 0);
    _phase = DATA_SEND_LOOP;
}

void HTTPResponseHandler::setHTMLHeader(const std::string& extension, const long contentLength) {
    setTypeHeader(getMIME(extension));
    setLengthHeader(contentLength);
    convertHeaderMapToString(false);
}


// NOTE:
// 1. root가 nginx.conf에 있는지 확인
//    -- 있음: 요청받은 파일/폴더명(data._URIFilePath)을 가지고오기
//            (ex. root: /User/yekim/, location: /data)
//    -- 없음: 에러 띄우기
// 2. 요청받은 파일/폴더명이 location 경로와 비교될 수 있도록 임시 경로 생성
// 3. 임시경로가 서버 컴퓨터에 존재하는지 확인
//    -- 있음: 폴더인지 파일인지 확인
//            -- 폴더인 경우: 4. nginx.conf server context의 location path를 순회
//            -- 파일인 경우: file에 대한 정보를 바로 반환 :: FINISH
//    -- 없음: 404 Not Found 페이지 열림 :: FINISH
// 4-1. location path와 일치하는 location인지 확인
//      -- 일치: 5. 해당 location의 context에서 정보를 가져옴
//      -- 불일치: 서버컴퓨터에는 존재하지만, nginx.conf에 세팅해두지 않은 경우, 404 Not Found 페이지 열림 :: FINISH
// 4-2. index 페이지 세팅
//      -- nginx.conf의 index가 있는지 확인 및 서버에 index가 있는지 확인
//         -- 있음: nginx.conf location context의 index 사용
//         -- 없음: _serverIndex를 그대로 사용.
// 4-3. _locIndex/_serverIndex가 서버 컴퓨터에 있는지 확인
//      -- 있음: _serverIndex 파일 전송 :: FINISH
//      -- 없음: autoindex가 켜져있는지 확인
//              -- on: autoindex 페이지 response :: FINISH
//              -- off: 403 에러

HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data) {
    if (_phase == FIND_RESOURCE) {
        // TODO: data 인수에 request 파싱된 결과가 들어있어서 이 클래스 초기화될때 data를 넣어서 초기화 하거나 여기서 초기화 해야합니다.
        _root = _serverConf.dirMap["root"];
        // nginx.conf에서 server index를 찾지 못할 경우, 기본 페이지를 index.html으로 설정
        _serverIndex = getIndexFile(_root + data._URIFilePath, _serverConf.index);
        _serverIndex = _serverIndex.empty() ? std::string("index.html") : _serverIndex;

        _type = FileController::checkType(_root + data._URIFilePath);
        std::cout << "[DEBUG] after first checkType: " << _type << std::endl;
        if (_type == FileController::NOTFOUND) { // 서버컴퓨터에 존재하지 않는 경우
            setGeneralHeader("HTTP/1.1 404 Not Found");
            data._statusCode = 404;
            _phase = NOT_FOUND;
        } else if (_type == FileController::DIRECTORY) {
            // NOTE: 해야할 것: location/server index 분석 및 autoindex 체크
            std::cout << "[DEBUG] path for searching file: " << _root + data._URIFilePath << std::endl;
            bool isLocFlag = false;
            NginxConfig::LocationBlock tmpLocConf;
            for (int i = 0; i < (int)_serverConf.location.size(); ++i) {
                if (_serverConf.location[i].locationPath == data._URIFilePath
                    || _serverConf.location[i].locationPath + "/" == data._URIFilePath) {
                    tmpLocConf = _serverConf.location[i];
                    isLocFlag = true;
                    break ;
                }
            }
            if (isLocFlag) {
                _locIndex = getIndexFile(_root + data._URIFilePath, tmpLocConf.index);
                _locIndex = _locIndex.empty() ? _serverIndex : _locIndex;
                _type = FileController::checkType(_root + data._URIFilePath + _locIndex);
                if (_type == FileController::FILE) {
                    setGeneralHeader("HTTP/1.1 200 OK");
                    data._statusCode = 200;
                    _absolutePath = _root + data._URIFilePath + _locIndex;
                    // FIXME: 리팩토링 때 수정해야할 항목
                    data._URIExtension = HTTPData::getExtension(_absolutePath);
                    _phase = GET_FILE;
                } else  {
                    if (tmpLocConf.dirMap["autoindex"] == "on") {
                        setGeneralHeader("HTTP/1.1 200 OK");
                        data._statusCode = 200;
                        _phase = AUTOINDEX;
                    } else {
                        setGeneralHeader("HTTP/1.1 403 Forbidden");
                        data._statusCode = 403;
                        _phase = NOT_FOUND;
                    }
                }
            } else {         // 서버컴퓨터에는 존재 하지만, nginx.conf에 세팅된 경로가 아닌 경우
                setGeneralHeader("HTTP/1.1 404 Not Found");
                data._statusCode = 404;
                _phase = NOT_FOUND;
            }
        } else if (_type == FileController::FILE) {
            setGeneralHeader("HTTP/1.1 200 OK");
            data._statusCode = 200;
            _absolutePath = _root + data._URIFilePath;
            data._URIExtension = HTTPData::getExtension(_absolutePath);
            _phase = GET_FILE;
        } 
    }
        
    if (_phase == NOT_FOUND) {
        responseNotFound(data);
    } else if (_phase == AUTOINDEX) {
        responseAutoIndex(data);
    } else if (_phase == TEST) {
        responseTest(data);
    }

    if (_phase == GET_FILE) {
        _file = new FileController(_absolutePath, FileController::READ);
        setHTMLHeader(data._URIExtension, _file->length());
        send(_connectionFd, _headerString.data(), _headerString.length(), 0);
        _phase = DATA_SEND_LOOP;
    }

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