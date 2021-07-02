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

std::string HTTPResponseHandler::getServerIndex(NginxConfig::ServerBlock server) {
    for (int i = 0; i < (int)server.index.size(); i++) {
        if (FileController::checkType(_absolutePath + server.index[i]) == FileController::FILE) {
            return (server.index[i]);
        }
    }
    // return (std::string("index.html"));
    return (std::string(""));
}

std::string HTTPResponseHandler::getIndexFile(const std::string& absolutePath, std::vector<std::string>& indexVec) {
    std::vector<std::string>::iterator iter;
    for (iter = indexVec.begin(); iter != indexVec.end(); ++iter) {
        std::cout << "[DEBUG] in getIndexFile: " << absolutePath + *iter << std::endl;
        if (FileController::checkType(absolutePath + *iter) == FileController::FILE) {
            return (*iter);
        }
    }
    // return (std::string("index.html"));
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


// NOTE:
// 1. root가 있는지 확인
//    -- 있음: 요청받은 파일/폴더명(data._URIFilePath)을 가지고오기
//            (ex. root: /User/yekim/, location: /data)
//    -- 없음: 에러 띄우기
// 2. 요청받은 파일/폴더명이 location 경로와 비교될 수 있도록 임시 경로 생성
// 3. 임시경로가 서버 컴퓨터에 존재하는지 확인
//    -- 있음: 폴더인지 파일인지 확인
//            -- 폴더인 경우: 4. nginx.conf server context의 location path를 순회
//            -- 파일인 경우: file에 대한 정보를 바로 반환 :: FINISH
//    -- 없음: 404 Not Found 페이지 열림 :: FINISH
// 4. location path와 일치하는 location인지 확인
//    -- 일치: 5. 해당 location의 context에서 정보를 가져옴
//    -- 불일치: 404 Not Found 페이지 열림 :: FINISH
// 5. location에 index 있는지 확인
//    -- 있음: _locIndex = _URILocPath + nginx.conf location context의 index
//    -- 없음: _locIndex = _URILocPath + _serverIndex

// NOTE:
// 1. data._URIFilePath가 서버 컴퓨터에 존재하는지 확인
//    -- 있음: 폴더인지 파일인지 확인
//            -- 폴더인 경우: location context의 경로에 포함되는지 확인
//            -- 파일인 경우:
//    -- 없음: 404 Not Found 페이지 열림
// 4. 아래의 동작 수행


// NOTE:
// 1. nginx.conf에 index -- 있음: 서버 폴더에 있는지 확인 -- 있음: conf의 index 사용
//                                                 -- 없음: 기본 값(index.html) 사용
//                 -- 없음: 기본 값(index.html) 사용
// 2. index가 -- 있음: 정상적으로 페이지 열행
//           -- 없음: autoindex -- on: autoindex 페이지 열림
//                             -- off: 403 Forbidden 페이지 열림

HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data) {
    if (_phase == FIND_RESOURCE) {
        // TODO: data 인수에 request 파싱된 결과가 들어있어서 이 클래스 초기화될때 data를 넣어서 초기화 하거나 여기서 초기화 해야합니다.
        std::cout << "Preprocessing DEBUGING======================================" << std::endl;
        _root = _serverConf.dirMap["root"];
        std::cout << "[DEBUG] _root: " << _root << std::endl;
        std::cout << "[DEBUG] data._URIFilePath: " << data._URIFilePath << std::endl;
        for (int i = 0; i < (int)_serverConf.location.size(); ++i) {
            std::cout << "[DEBUG] _loation[" << i << "] path: " << _serverConf.location[i].locationPath << std::endl;
            std::cout << "[DEBUG] _loation[" << i << "] autoindex: " << _serverConf.location[i].dirMap["autoindex"] << std::endl;
        }
        _absolutePath = _root + data._URIFilePath;
        data._reqAbsoluteFilePath = _root + data._URIFilePath;
        // std::cout << "[DEBUG] _absolutePath" << _absolutePath << std::endl;
        _serverIndex = getIndexFile(_absolutePath, _serverConf.index);
        // std::cout << "[DEBUG] _serverIndex" << _serverIndex << std::endl;

        std::cout << "[DEBUG] path for searching file: " << _root + data._URIFilePath << std::endl;
        _type = FileController::checkType(_root + data._URIFilePath);
        if (_type == FileController::NOTFOUND) {
            setGeneralHeader("HTTP/1.1 404 Not Found");
            data._statusCode = 404;
            _phase = NOT_FOUND;
        } else if (_type == FileController::DIRECTORY) {
            _phase = FINISH;
        } else if (_type == FileController::FILE) {
            _phase = FINISH;
        } 
        std::vector<struct NginxConfig::LocationBlock>::iterator iter;
        for (iter = _serverConf.location.begin(); iter != _serverConf.location.end(); ++iter) {
            _locIndex = getIndexFile(_absolutePath, iter->index);
            std::cout << "[DEBUG] _locIndex: " << _locIndex << std::endl;
        }
    }
        
    if (_phase == NOT_FOUND) {
        responseNotFound(data);
    } 
    if (_phase == TEST) {
        responseAutoIndex(data);
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

    //     _type = FileController::checkType(_root + data._reqFilePath);
    //     data._reqAbsoluteFilePath = _absolutePath + _serverIndex;

    //     if (_type == FileController::NOTFOUND) {
    //         setGeneralHeader("HTTP/1.1 404 Not Found");
    //         _phase = NOT_FOUND;
    //     } else if (_type == FileController::DIRECTORY) {
    //         setGeneralHeader("HTTP/1.1 200 OK");
    //         if (_serverIndex.empty()) { // 만약 _serverIndex가 없다면 바로 auto index로 띄우기
    //             _phase = AUTOINDEX;
    //         } else {                    // 만약 dierctory이고 server index가 있다면 절대 경로에 index 더하기
    //             _absolutePath = _absolutePath + _serverIndex;
    //             _phase = GET_FILE;
    //         }
    //     } else if (_type == FileController::FILE) {
    //         setGeneralHeader("HTTP/1.1 200 OK");
    //         if (isCGI(data._URIExtension)) {
    //             _phase = CGI_RUN;
    //         } else {
    //             _phase = GET_FILE;
    //         }
    //     }
    // } 

#if 0
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