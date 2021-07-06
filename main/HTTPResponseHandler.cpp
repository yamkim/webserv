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

std::string HTTPResponseHandler::getMIME(const std::string& extension) const {
    std::map<std::string, std::string> mime = _nginxConf._http.types.typeMap;
    if(extension == std::string("") || mime.find(extension) == mime.end()) {
        return (mime["default_type"]);
    } else {
        return (mime[extension]);
    }
}

void HTTPResponseHandler::setHTMLHeader(const HTTPData& data) {
    std::stringstream ssLength;
    ssLength << data._resContentLength;
    _headers["Content-Type"] = getMIME(data._URIExtension);
    _headers["Content-Length"] = ssLength.str();
    if (data._statusCode == 301) {
        _headers["Location"] = data._resAbsoluteFilePath;
    }
    convertHeaderMapToString(false);
    std::cout << "[DEBUG] headers: " << _headerString << std::endl;
}

// NOTE:
// 1. root가 nginx.conf에 있는지 확인
//    -- 있음: 요청받은 파일/폴더명(data._URIFilePath)을 가지고오기
//            (ex. root: /User/yekim/, location: /data)
//    -- 없음: 에러 띄우기 (아직 구현 안됨)
// 2. cgi 관련 정보를 맵[key: 사용할 확장자명, value: binary pass 형태로 얻어두기 (참고 1.)
// 3. 요청받은 파일/폴더명을 모든 nginx.conf의 location 경로들과 비교하기 
//    (일단 해당하는 req uri가 어떤 location block을 사용할지가 가장 중요!)
//    # 가장 일치하는 로케이션 블록의 설정을 받아오게 될 것임.
//    # 만약 location /data만 있다면, localhost:4242/data/a도 /data로 받아오게 됨. (파일이든 폴더든 상관 x)
// 4. location 블록이 설정됨
// 5. redirection 설정이 location block에 있는지 확인
//    -- 있음: 바로 리다이렉션 시키기
//    -- 없음: 6. request uri type 판단
// 6. request uri의 type을 판단
//    -- 폴더: 끝에 '/'가 붙었는지 확인
//            -- 없음: '/'를 끝에 가지도록 리다이렉션
//            -- 있음: nginx.conf에 server index가 설정되었는지 확인
//                    -- 됨: location index가 설정되었는지 확인
//                          -- 됨: * server index 관련 설정 무시하고 location index로 덮어쓰기
//                                 * 서버 컴퓨터에 있는 index 파일인지 확인
//                                   -- 있음: 파싱한 index에 대한 파일이 cgi인지 확인 -- (a)
//                                           -- cgi임: phase = GET_CGI_RUN
//                                           -- cgi아님: phase = GET_FILE
//                                   -- 없음: 403 에러 띄우기 (에러페이지에 대한 설정 같이 넣기)
//                          -- 안됨: server index 사용하기
//                    -- 안됨: autoindex 설정 되었는지 확인
//                            -- 됨: autoindex 페이지 띄우기
//                            -- 안됨: 403 에러 띄우기 (에러페이지에 대한 설정 같이 넣기)
//    -- 파일: cgi인지 확인하기 -- (a)
//            -- cgi임: phase = GET_CGI_RUN
//            -- cgi아님: phase = GET_FILE
//    -- 없는: 404 에러 띄우기 (에러페이지에 대한 설정 같이 넣기)
// @ error_page: nginx.conf에서 error_page 설정이 있는지 확인
//               -- 있음: 해당 위치의 파일을 가져오기 => phase = GET_FILE
//               -- 없음: 정적 파일 가져오기 => phase = GET_STATIC_HTML
//   (ex. error_page  403 404 405 406 411 497 500 501 502 503 504 505 /error.html;)
// 참고 사항
// 1. cgi의 경우, 다른 location context에서 정의되어도 index로 사용 가능
// 2. 


// NOTE: cgi 파트 location 블록과 함께 사용하기
// 1. req uri 경로에서 확장자 파싱하기
// 2. 확장자가 location path를 순환하며 얻은 경로와 일치하는지 파악하기
//      -- 일치: 7. cgi 작업 전 필요한 변수 세팅하기
//      -- 불일치: phase = GET_FILE
// 3. 해당 location 블록 내부에서 바이너리 파일 위치 가져오기
// 4. phase = CGI_RUN으로 변경하기

// TODO
// 1. try_files, return, deny 부분 추가하기
// 5. return 관련부분, cgi pass
// - request로부터 오는 dataStatus에 따라서 먼저 출력하는 방법 모색 -> 생성자에서 처리?

HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data) {
    if (_phase == FIND_RESOURCE) {
        // 1
        data._root = _serverConf.dirMap["root"];
        _serverIndex = getIndexFile(data._root + data._URIFilePath, _serverConf.index);
        // 2
        for (std::size_t i = 0; i < _serverConf.location.size(); ++i) {
            std::string tmpExt = HTTPData::getExtension(_serverConf.location[i].locationPath);
            NginxConfig::LocationBlock tmpLocBlock = _serverConf.location[i];
            if (tmpExt.empty())
                continue ;
            // std::cout << "[DEBUG] getMapValue[" << tmpExt << "]: " << tmpLocBlock.dirMap["cgi_pass"] << std::endl;
            _cgiConfMap[tmpExt] = tmpLocBlock.dirMap["cgi_pass"];
        }


#if 0

        // TODO: Redirection 완벽하게 구현 후, 슬래시가 뒤에 붙지 않은 폴더의 경우 리다이렉션으로 돌려주기
        // TODO: location / 블록 없더라도 index.html이 켜지도록 동작해야함
        _type = FileController::checkType(data._root + data._URIFilePath);
        if (_type == FileController::DIRECTORY && data._URIFilePath[data._URIFilePath.size() - 1] == '/') {
            // 현재 방식: location의 path와 정확히 일치하는 경로의 block만 취급
            bool isLocFlag = false;
            NginxConfig::LocationBlock tmpLocConf;
            std::size_t matchLen = 0; 

            std::cout << "[DEBUG] data._URIFilePath: [" << data._URIFilePath << "]" << std::endl;
            for (std::size_t i = 0; i < _serverConf.location.size(); ++i) {
                std::string tmpLocPath = _serverConf.location[i].locationPath;
                if (data._URIFilePath == std::string("/") && tmpLocPath == std::string("/")) {
                    isLocFlag = true;
                    tmpLocConf = _serverConf.location[i];
                    break ;
                } else {
                    // NOTE: GUIDE LINE for Prefix Match
                    // - request uri로 /data/a가 들어오는 경우
                    // 1. location path를 순환하며, path가 request uri의 첫부분에 포함되는지 찾기
                    // 2. 만약 포함 된다면, 길이를 기억하기
                    // 3. 가장 긴 길이(가장 알맞은 경로)의 path를 갖는 location block을 사용하기
                    // 주의: 폴더여야하므로 항상 끝은 "/"로 끝나도록
                    //      (ex. location /data/ab/ 인데, req uri: /data/a인 경우는 폴더가 아닌 파일)
                    // case 1. localhost:4242/data 받으면: localhost:4242/data/ 가 아니므로 오류..
                    // case 2. location path에 /data만 있는 경우에, /data/a를 받아도 에러가 나지 않도록 설정하기
                    // case 3. localhost:4242/data는 불가능 localhost:4242/data/만 가능
                    tmpLocPath = tmpLocPath + std::string("/");
                    std::size_t j = 0;
                    for (; j < data._URIFilePath.size(); ++j) {
                        if (tmpLocPath[j] != data._URIFilePath[j]) {
                            break ;
                        }
                    }
                    if (j > 0 && (data._URIFilePath[j - 1] == '/' && matchLen < j)) {
                        isLocFlag = true;
                        matchLen = j;
                        tmpLocConf = _serverConf.location[i];
                    }
                }
            }
            if (isLocFlag) {                    
                // NOTE GUIDE LINE FOR REDIRECTION
                // 1. tmpLocConf._return이 있는지 확인
                //    -- 있음: 2. status code, path 파싱
                //    -- 없음: index 확인 부로 바로 넘어가기
                // 2-1. statusCode = tmpLocConf._return[0]
                // 2-2. absolutePath = tmpLocConf._return[1]
                if (!tmpLocConf._return.empty()){
                    // FIXME: setGeneralHeader에 들어갈 string도 status code에 따라서 처리하기
                    // setGeneralHeader("HTTP/1.1 301 Moved Permanently");
                    setGeneralHeader("HTTP/1.1 302 Found");
                    data._statusCode = atoi(tmpLocConf._return[0].c_str());
                    data._resAbsoluteFilePath = tmpLocConf._return[1];
                    data._URIExtension = "html";
                    std::cout << "[DEBUG] Redirection Case: status code: " << data._statusCode << std::endl;
                    std::cout << "[DEBUG] Redirection Case: absolute path: " << data._resAbsoluteFilePath << std::endl;
                    _phase = GET_STATIC_HTML;
                } else { // index 확인부
                    // NOTE: location에 index가 없는 경우, server index로부터 상속
                    // NOTE: location에 index가 있는 경우, server index 무시하고 새로 확인
                    if (tmpLocConf.index.empty()) {
                        _locIndex = _serverIndex;
                    } else { 
                        _locIndex = getIndexFile(data._root + data._URIFilePath, tmpLocConf.index);
                    }
                    _type = FileController::checkType(data._root + data._URIFilePath + _locIndex);
                    if (!_locIndex.empty() && _type == FileController::FILE) {
                        setGeneralHeader("HTTP/1.1 200 OK");
                        data._statusCode = 200;
                        data._resAbsoluteFilePath = data._root + data._URIFilePath + _locIndex;
                        data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                        std::cout << "[DEBUG] open location index file: " << data._resAbsoluteFilePath << std::endl;
                        if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
                            data._CGIBinary = _cgiConfMap[data._URIExtension];
                            _phase = CGI_RUN;
                        } else {
                            _phase = GET_FILE;
                        }
                    } else  {
                        // location block에 autoindex 없으면 상속받기
                        if (tmpLocConf.dirMap["autoindex"].empty()) {
                            tmpLocConf.dirMap["autoindex"] = _serverConf.dirMap["autoindex"];
                        }
                        if (tmpLocConf.dirMap["autoindex"] == "on") {
                            setGeneralHeader("HTTP/1.1 200 OK");
                            data._statusCode = 200;
                        } else {
                            setGeneralHeader("HTTP/1.1 403 Forbidden");
                            data._statusCode = 403;
                        }
                        data._URIExtension = "html";
                        _phase = GET_STATIC_HTML;
                    }
                }
            } else {         // 서버컴퓨터에는 존재 하지만, nginx.conf에 세팅된 경로가 아닌 경우
                std::cout << "[DEBUG] exist in Server not nginx config" << std::endl;
                setGeneralHeader("HTTP/1.1 404 Not Found");
                data._statusCode = 404;
                data._URIExtension = "html";
                _phase = GET_STATIC_HTML;
            }
        } else if (_type == FileController::FILE) {
            setGeneralHeader("HTTP/1.1 200 OK");
            data._statusCode = 200;
            data._resAbsoluteFilePath = data._root + data._URIFilePath;
            data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
            std::cout << "[DEBUG] open location file (not index): " << data._resAbsoluteFilePath << std::endl;
            if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
                data._CGIBinary = _cgiConfMap[data._URIExtension];
                _phase = CGI_RUN;
            } else {
                _phase = GET_FILE;
            }
        } else { // 서버컴퓨터에 존재하지 않는 경우
            setGeneralHeader("HTTP/1.1 404 Not Found");
            data._statusCode = 404;
            _phase = GET_STATIC_HTML;
        } 
#endif
    }

    if (_phase == GET_STATIC_HTML) {
        _staticHtml = HTMLBody::getStaticHTML(data);
        // setHTMLHeader(data._URIExtension, _staticHtml.length());
        data._resContentLength = _staticHtml.length();
        setHTMLHeader(data);
        send(_connectionFd, _headerString.data(), _headerString.length(), 0);
        _phase = DATA_SEND_LOOP;
    } else if (_phase == GET_FILE) {
        _file = new FileController(data._resAbsoluteFilePath, FileController::READ);
        data._resContentLength = _file->length();
        setHTMLHeader(data);
        send(_connectionFd, _headerString.data(), _headerString.length(), 0);
        _phase = DATA_SEND_LOOP;
    }

    if (_phase == DATA_SEND_LOOP) {
        char *buf;
        size_t writtenLengthOnBuf;
        if (_staticHtml.empty()) {
            buf = new char[RESPONSE_BUFFER_SIZE];
            writtenLengthOnBuf = read(_file->getFd(), buf, RESPONSE_BUFFER_SIZE);
            _phase = writtenLengthOnBuf == 0 ? FINISH : DATA_SEND_LOOP;
        } else {
            buf = new char[data._resContentLength + 1];
            buf[data._resContentLength] = '\0';
            writtenLengthOnBuf = strlcpy(buf, _staticHtml.c_str(), data._resContentLength + 1);
            _phase = FINISH;
        }
        size_t writtenLengthOnSocket = send(_connectionFd, buf, writtenLengthOnBuf, 0);
        if (writtenLengthOnSocket != writtenLengthOnBuf) {
            delete [] buf;
            throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
        }
        delete [] buf;
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