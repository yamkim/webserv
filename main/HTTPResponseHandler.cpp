#include "HTTPResponseHandler.hpp"

// FIXME : 리스폰스에 URI 외에 더 다양한 아규먼트를 집어넣어야 하는데 어떤 형식으로 집어넣을지 고민 중입니다. 추후에 수정하겠습니다.
HTTPResponseHandler::HTTPResponseHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, const NginxConfig& nginxConf) : HTTPHandler(connectionFd, serverConf, nginxConf) {
    _phase = FIND_RESOURCE;
    _file = NULL;
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

std::string HTTPResponseHandler::getMIME(const std::string& extension) const {
    std::map<std::string, std::string> mime = _nginxConf._http.types.typeMap;
    if(extension == std::string("") || mime.find(extension) == mime.end()) {
        return (mime["default_type"]);
    } else {
        return (mime[extension]);
    }
}

void HTTPResponseHandler::setHTMLHeader(const std::string& extension, const long contentLength) {
    _headers["Content-Type"] = getMIME(extension);
    std::stringstream ssLength(contentLength);
    _headers["Content-Length"] = ssLength.str();
    convertHeaderMapToString(false);
}

// NOTE:
// 1. root가 nginx.conf에 있는지 확인
//    -- 있음: 요청받은 파일/폴더명(data._URIFilePath)을 가지고오기
//            (ex. root: /User/yekim/, location: /data)
//    -- 없음: 에러 띄우기
// 2. cgi 관련 정보를 맵[key: 사용할 확장자명, value: binary pass 형태로 얻어두기
// 3. 요청받은 파일/폴더명이 location 경로가 서버 컴퓨터에 존재하는지 확인
//    -- 있음: 폴더인지 파일인지 확인
//            -- 폴더인 경우: 4. nginx.conf server context의 location path를 순회
//            -- 파일인 경우: cgi인지 확인하기
//                         -- cgi인 경우: 6. cgi에 대한 처리
//                         -- cgi가 아닌 경우: file에 대한 정보를 바로 반환 :: FINISH
//    -- 없음: 404 Not Found 페이지 열림 :: FINISH
// 4-1. location path와 일치하는 location인지 확인
//      -- 일치: 5. 해당 location의 context에서 정보를 가져옴
//      -- 불일치: 서버컴퓨터에는 존재하지만, nginx.conf에 세팅해두지 않은 경우, 404 Not Found 페이지 열림 :: FINISH
// 4-2. index 페이지 세팅 -> TODO 만약, index로 cgi 파일이 사용되는 경우 추가
//      -- nginx.conf의 index가 있는지 확인 및 서버에 index가 있는지 확인
//         -- 있음: nginx.conf location context의 index 사용 
//                 (cgi인 경우에는 다른 location 블록에 cgi가 적용됐더라도 사용 가능)
//         -- 없음: _serverIndex를 그대로 사용.
// 4-3. _locIndex/_serverIndex가 서버 컴퓨터에 있는지 확인
//      -- 있음: cgi인지 확인하기
//              -- cgi인 경우: 6. cgi에 대한 처리
//              -- cgi가 아닌 경우: _serverIndex 파일 전송 :: FINISH
//      -- 없음: autoindex가 켜져있는지 확인
//              -- on: autoindex 페이지 response :: FINISH
//              -- off: 403 에러
// NOTE: cgi 파트 location 블록과 함께 사용하기
// 6-1. req uri 경로에서 확장자 파싱하기
// 6-2. 확장자가 location path를 순환하며 얻은 경로와 일치하는지 파악하기
//      -- 일치: 7. cgi 작업 전 필요한 변수 세팅하기
//      -- 불일치: phase = GET_FILE
// 7-1. 해당 location 블록 내부에서 바이너리 파일 위치 가져오기
// 7-2. phase = CGI_RUN으로 변경하기

// error_page directive 추가하기
// 1. nginx.conf에 추가 후, vector로 파싱하기
//    (ex. error_page  403 404 405 406 411 497 500 501 502 503 504 505 /error.html;)
// 2. 

// TODO
// 1. try_files, return, deny 부분 추가하기
// 2. location 블록에 .py, .php 부분 추가하기
// 3. autoindex 체크하는 부분 server context 레벨까지 끌어올리기
// 4. 기본적으로 prefix match로 사용
// 5. return 관련부분, cgi pass

HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data) {
    if (_phase == FIND_RESOURCE) {
        _root = _serverConf.dirMap["root"];
        _serverIndex = getIndexFile(_root + data._URIFilePath, _serverConf.index);

        // NOTE: CGI로 사용될 파일 확장자 parsing
        for (int i = 0; i < (int)_serverConf.location.size(); ++i) {
            std::string tmpExt = HTTPData::getExtension(_serverConf.location[i].locationPath);
            NginxConfig::LocationBlock tmpLocBlock = _serverConf.location[i];
            if (tmpExt.empty())
                continue ;
            std::cout << "[DEBUG] getMapValue[" << tmpExt << "]: " << tmpLocBlock.dirMap["cgi_pass"] << std::endl;
            _cgiConfMap[tmpExt] = tmpLocBlock.dirMap["cgi_pass"];
        }

        _type = FileController::checkType(_root + data._URIFilePath);
        if (_type == FileController::NOTFOUND) { // 서버컴퓨터에 존재하지 않는 경우
            setGeneralHeader("HTTP/1.1 404 Not Found");
            data._statusCode = 404;
            _phase = NOT_FOUND;
        } else if (_type == FileController::DIRECTORY) {
            // NOTE: 해야할 것: location/server index 분석 및 autoindex 체크
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
                // NOTE: location에 index가 없는 경우, server index로부터 상속
                // NOTE: location에 index가 있는 경우, server index 무시하고 새로 확인
                if (tmpLocConf.index.empty()) {
                    _locIndex = _serverIndex;
                } else { 
                    _locIndex = getIndexFile(_root + data._URIFilePath, tmpLocConf.index);
                }
                _type = FileController::checkType(_root + data._URIFilePath + _locIndex);
                if (!_locIndex.empty() && _type == FileController::FILE) {
                    setGeneralHeader("HTTP/1.1 200 OK");
                    data._statusCode = 200;
                    data._resAbsoluteFilePath = _root + data._URIFilePath + _locIndex;
                    data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                    std::cout << "[DEBUG] open location index file: " << data._resAbsoluteFilePath << std::endl;
                    if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
                        data._CGIBinary = _cgiConfMap[data._URIExtension];
                        _phase = CGI_RUN;
                    } else {
                        _phase = GET_FILE;
                    }
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
                std::cout << "[DEBUG] exist in Server not nginx config" << std::endl;
                setGeneralHeader("HTTP/1.1 404 Not Found");
                data._statusCode = 404;
                _phase = NOT_FOUND;
            }
        } else if (_type == FileController::FILE) {
            setGeneralHeader("HTTP/1.1 200 OK");
            data._statusCode = 200;
            data._resAbsoluteFilePath = _root + data._URIFilePath;
            data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
            std::cout << "[DEBUG] open location file (not index): " << data._resAbsoluteFilePath << std::endl;
            if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
                data._CGIBinary = _cgiConfMap[data._URIExtension];
                _phase = CGI_RUN;
            } else {
                _phase = GET_FILE;
            }
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
        _file = new FileController(data._resAbsoluteFilePath, FileController::READ);
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

int HTTPResponseHandler::getCGIfd(void) {
    return (_cgi->getOutputStream());
}