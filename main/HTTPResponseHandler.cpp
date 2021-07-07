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

std::string HTTPResponseHandler::getErrorPage(const std::string& absolutePath, std::vector<std::string>& errorPageVec) {
    std::vector<std::string>::iterator iter = errorPageVec.end() - 1;
    std::cout << "[DEBUG] error_page from error_page list: " << *iter << std::endl;
    std::cout << "[DEBUG] root + error_page from error_page list: " << absolutePath + *iter << std::endl;
    if (FileController::checkType(absolutePath + *iter) == FileController::FILE) {
        return (*iter);
    }
    return (std::string(""));
}

bool HTTPResponseHandler::isErrorPageList(int statusCode, std::vector<std::string>& errorPageVec) {
    std::stringstream ssStatusCode;
    ssStatusCode << statusCode;
    std::cout << "[DEBUG] statusCode: " << ssStatusCode.str() << std::endl;  
    if (find(errorPageVec.begin(), errorPageVec.end(), ssStatusCode.str()) != errorPageVec.end()) {
        std::cout << "[DEBUG] Error Page is searched in errorPageList============" << std::endl;
        return true;
    }
    std::cout << "[DEBUG] Error Page is not searched in errorPageList============" << std::endl;
    return false;
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
// 2. server 블록에서 사용할 index, error_page 받아두기
// 3. 요청받은 파일/폴더명을 모든 nginx.conf의 location 경로들과 비교하기 
//    (일단 해당하는 req uri가 어떤 location block을 사용할지가 가장 중요!)
//    # 가장 일치하는 로케이션 블록의 설정을 받아오게 될 것임.
// 4. 매칭되는 location 블록이 설정됨
//    -- 블록 있음: 5. 블록 설정에 따라 진행
//    -- 블록 없음: 추후 고민=============================
// 5. error시 반환해줄 page 세팅
// 5. request uri의 type을 판단
//    -- 폴더: 끝에 '/'가 붙었는지 확인
//            -- 없음: '/'를 끝에 가지도록 리다이렉션
//            -- 있음: redirection 설정이 되어있는지 확인
//                    -- 됨: 바로 리다이렉션 시키기             
//                    -- 안됨: nginx.conf에 server index가 설정되었는지 확인
//                            -- 됨: location index가 설정되었는지 확인
//                                  -- 됨: server index 관련 설정 무시하고 location index로 덮어쓰기
//                                  -- 안됨: server index 사용하기
//                                  서버 컴퓨터에 있는 index 파일인지 확인
//                                  -- 있음: 파싱한 index에 대한 파일이 cgi인지 확인 -- (a)
//                                          -- cgi임: phase = GET_CGI_RUN
//                                          -- cgi아님: phase = GET_FILE
//                                  -- 없음: 403 에러 띄우기 (에러페이지에 대한 설정 같이 넣기)
//                            -- 안됨: autoindex 설정 되었는지 확인
//                                    -- 됨: autoindex 페이지 띄우기
//                                    -- 안됨: 403 에러 띄우기 (에러페이지에 대한 설정 같이 넣기)
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
    // TODO: 해당하는 로케이션의 설정에서 status 코드 적용시키기
    // if (data._statusCode == 400 || data._statusCode == 413) {
    //     if (data._statusCode == 400) {
    //         std::cout << "[DEBUG] HTTP/1.1 400 error test" << std::endl;
    //         setGeneralHeader("HTTP/1.1 400 Not Found");
    //         data._URIExtension = "html";
    //         _phase = GET_STATIC_HTML;
    //     } else if (data._statusCode == 413) {
    //         std::cout << "[DEBUG] HTTP/1.1 413 error test" << std::endl;
    //         setGeneralHeader("HTTP/1.1 413 Payload Too Large");
    //         data._URIExtension = "html";
    //         _phase = GET_STATIC_HTML;
    //     }
    // }
    if (_phase == FIND_RESOURCE) {
        // 1
        data._root = _serverConf.dirMap["root"];
        _serverIndex = getIndexFile(data._root + data._URIFilePath, _serverConf.index);
        // server block에 error_page 세팅이 있는 경우
        // -- 있음: error_page 찾아서 에러 파일이 존재하는지 찾기
        //         -- 존재: error_page로 사용
        //         -- 존재안함: error_page는 ""
        // -- 없음: error_page는 ""
        if (!_serverConf.error_page.empty()) {
            _errorPageList = _serverConf.error_page;
            _serverErrorPage = getErrorPage(data._root, _serverConf.error_page);
        }

        // 2
        for (std::size_t i = 0; i < _serverConf.location.size(); ++i) {
            std::string tmpExt = HTTPData::getExtension(_serverConf.location[i].locationPath);
            NginxConfig::LocationBlock tmpLocBlock = _serverConf.location[i];
            if (tmpExt.empty())
                continue ;
            // std::cout << "[DEBUG] getMapValue[" << tmpExt << "]: " << tmpLocBlock.dirMap["cgi_pass"] << std::endl;
            _cgiConfMap[tmpExt] = tmpLocBlock.dirMap["cgi_pass"];
        }
        std::cout << "[DEBUG] data._URIFilePath: [" << data._URIFilePath << "]" << std::endl;

        // 3
        bool isLocFlag = false;
        NginxConfig::LocationBlock locConf;
        std::size_t matchLen = 0; 
        for (std::size_t i = 0; i < _serverConf.location.size(); ++i) {
            // NOTE: GUIDE LINE for Prefix Match
            // - request uri와 가장 일치하는 location 블록의 설정을 사용하되, 접근은 모두 request uri 기준으로하기!
            // 1. location path를 순환하며, path가 request uri의 앞부분에 포함되는지 찾기
            // 2. 만약 포함 된다면, 길이를 기억하기
            // 3. 가장 긴 길이(가장 알맞은 경로)의 path를 갖는 location block의 설정을 사용하기
            // 주의: 폴더/파일인지는 일단 상관 없음..! 여야하므로 항상 끝은 "/"로 끝나도록
            //      (ex. location /data/ab/ 인데, req uri: /data/a인 경우는 폴더가 아닌 파일)
            std::string tmpLocPath = _serverConf.location[i].locationPath;
            std::size_t j = 0; // 1인 경우는 / 일 때이므로
            for (; j < data._URIFilePath.size(); ++j) {
                if (tmpLocPath[j] != data._URIFilePath[j]) {
                    break ;
                }
            }
            if (j == 1) { 
                if (tmpLocPath == "/") { // 하나만 일치하는 경우, tmpLocPath가 "/"가 아니면 일치하는 location 블록이 없는 것.
                    isLocFlag = true;
                    matchLen = j;
                    locConf = _serverConf.location[i];
                }
            } else {
                if (matchLen < j) { // 더 많은 글자가 일치하는 location 블록이 있는 경우
                    isLocFlag = true;
                    matchLen = j;
                    locConf = _serverConf.location[i];
                }
            }
        }
        std::cout << "[DEBUG] isLocFlag: " << isLocFlag << std::endl;
        std::cout << "[DEBUG] locConf.locationPath: " << locConf.locationPath << std::endl;
        if (isLocFlag) { // 4
            // index page 세팅 및 error page 세팅
            _locIndex = locConf.index.empty()
                        ? _serverIndex
                        : getIndexFile(data._root + data._URIFilePath, locConf.index);

            // location block에 error_page 세팅이 있는 경우
            // -- 있음: error_page 찾아서 에러 파일이 존재하는지 찾기
            //         -- 존재: error_page로 사용
            //         -- 존재안함: error_page는 ""
            // -- 없음: serverBlock의  error_page가 있나?
            //         -- 있음: server error_page 사용, _errorPageList는 server꺼로 사용
            //         -- 없음: error_page는 "", _errorPageList도 ""
            if (!locConf.error_page.empty()) {
                _errorPageList = locConf.error_page;
                _locErrorPage = getIndexFile(data._root, locConf.error_page);
            } else {
                if (!_serverErrorPage.empty()) {
                    _errorPageList = _serverConf.error_page;
                    _locErrorPage = _serverErrorPage;
                } else {
                    _locErrorPage = "";
                }
            }
            std::cout << "[DEBUG] location error_page path: ===========" << _locErrorPage << std::endl;
            _type = FileController::checkType(data._root + data._URIFilePath);
            // if (_type == FileController::DIRECTORY && data._URIFilePath[data._URIFilePath.size() - 1] == '/') {
            if (_type == FileController::DIRECTORY) {
                // TODO: 끝에 "/"가 붙어있는지 확인하기
                if (data._URIFilePath[data._URIFilePath.size() - 1] != '/') {
                    setGeneralHeader("HTTP/1.1 301 Moved Permanently");
                    // setGeneralHeader("HTTP/1.1 302 Found");
                    data._statusCode = 301;
                    data._resAbsoluteFilePath = data._URIFilePath + "/";
                    data._URIExtension = "html";
                    std::cout << "[DEBUG] Directory without '/' is redirected to: " << data._resAbsoluteFilePath << std::endl;
                    _phase = GET_STATIC_HTML;
                } else {
                    if (!locConf._return.empty()) { // redirection 시키는 것이 가장 우선순위가 높음
                    setGeneralHeader("HTTP/1.1 301 Moved Permanently");
                    // setGeneralHeader("HTTP/1.1 302 Found");
                    data._statusCode = atoi(locConf._return[0].c_str());
                    data._resAbsoluteFilePath = locConf._return[1];
                    data._URIExtension = "html";
                    std::cout << "[DEBUG] Redirection Case: status code: " << data._statusCode << std::endl;
                    std::cout << "[DEBUG] Redirection Case: absolute path: " << data._resAbsoluteFilePath << std::endl;
                    _phase = GET_STATIC_HTML;
                    } else { 
                        if (!_locIndex.empty()) { // index file이 어떻게든 있는 경우
                            // index 파일이 서버 컴퓨터에 있는지 판별
                            FileController::Type indexType = FileController::checkType(data._root + data._URIFilePath + _locIndex);
                            if (indexType == FileController::FILE) {
                                setGeneralHeader("HTTP/1.1 200 OK");
                                data._statusCode = 200;
                                data._resAbsoluteFilePath = data._root + data._URIFilePath + _locIndex;
                                data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                                std::cout << "[DEBUG] open location index file: " << data._resAbsoluteFilePath << std::endl;
                                // 인덱스 파일이 cgi 파일인지 판별
                                if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
                                    data._CGIBinary = _cgiConfMap[data._URIExtension];
                                    _phase = CGI_RUN;
                                } else {
                                    _phase = GET_FILE;
                                }
                            } else {
                                data._statusCode = 403;
                                if (isErrorPageList(data._statusCode, _errorPageList)) {
                                    setGeneralHeader("HTTP/1.1 200 OK");
                                    data._statusCode = 200;
                                    data._resAbsoluteFilePath = data._root + _locErrorPage;
                                    data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                                    std::cout << "[DEBUG] open location error_page file: " << data._resAbsoluteFilePath << std::endl;
                                    _phase = GET_FILE;
                                } else {
                                    setGeneralHeader("HTTP/1.1 403 Forbidden");
                                    data._URIExtension = "html";
                                    _phase = GET_STATIC_HTML;
                                }
                            }
                        } else {                  // index file이 어디에도 설정되지 않은 경우
                            locConf.dirMap["autoindex"] = locConf.dirMap["autoindex"].empty()
                                                        ? _serverConf.dirMap["autoindex"]
                                                        : locConf.dirMap["autoindex"];
                            if (locConf.dirMap["autoindex"] == "on") {
                                setGeneralHeader("HTTP/1.1 200 OK");
                                data._statusCode = 200;
                                data._URIExtension = "html";
                                _phase = GET_STATIC_HTML;
                            } else {
                                data._statusCode = 403;
                                if (isErrorPageList(data._statusCode, _errorPageList)) {
                                    setGeneralHeader("HTTP/1.1 200 OK");
                                    data._statusCode = 200;
                                    data._resAbsoluteFilePath = data._root + _locErrorPage;
                                    data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                                    std::cout << "[DEBUG] open location error_page file: " << data._resAbsoluteFilePath << std::endl;
                                    _phase = GET_FILE;
                                } else {
                                    setGeneralHeader("HTTP/1.1 403 Forbidden");
                                    data._URIExtension = "html";
                                    _phase = GET_STATIC_HTML;
                                }
                            }
                        }
                    }

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
            } else {
                data._statusCode = 404;
                if (isErrorPageList(data._statusCode, _errorPageList)) {
                    setGeneralHeader("HTTP/1.1 200 OK");
                    data._statusCode = 200;
                    data._resAbsoluteFilePath = data._root + _locErrorPage;
                    data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                    std::cout << "[DEBUG] open location error_page file: " << data._resAbsoluteFilePath << std::endl;
                    _phase = GET_FILE;
                } else {
                    setGeneralHeader("HTTP/1.1 404 Not Found");
                    data._URIExtension = "html";
                    _phase = GET_STATIC_HTML;
                }
            }
        } else {
            if (isErrorPageList(data._statusCode, _errorPageList)) {
                setGeneralHeader("HTTP/1.1 200 OK");
                data._statusCode = 200;
                data._resAbsoluteFilePath = data._root + data._URIFilePath + _locErrorPage;
                data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                std::cout << "[DEBUG] open location error_page file: " << data._resAbsoluteFilePath << std::endl;
                _phase = GET_FILE;
            } else {
                setGeneralHeader("HTTP/1.1 404 Not Found");
                data._statusCode = 404;
                data._URIExtension = "html";
                _phase = GET_STATIC_HTML;
            }
        }
    }

    if (_phase == GET_STATIC_HTML) {
        _staticHtml = HTMLBody::getStaticHTML(data);
        data._resContentLength = _staticHtml.length();
        std::cout << "DEBUG======================================" << std::endl;
        std::cout << "[DEBUG] _staticHtml: " << _staticHtml << std::endl;
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