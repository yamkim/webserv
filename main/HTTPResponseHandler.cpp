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

// NOTE location index 세팅을 위한 GUIDE LINE
// 1. nginx.conf에 server에 index 세팅이 있는지 확인
//    -- 있음: server index가 컴퓨터에 있는지 확인
//            -- 있음: serverIndex = data._root + data._URIFilePath + *iter
//            -- 없음: serverIndex = ""
//    -- 없음: serverIndex = ""
// 2. nginx.conf에 location에 대한 세팅이 있는지 확인
//    -- 있음: location index가 컴퓨터에 있는지 확인
//            -- 있음: locationIndex = data._root + data._URIFilePath + *iter
//            -- 없음: ""
//    -- 없음: locationIndex = serverIndex
std::string HTTPResponseHandler::getIndexPage(const HTTPData& data, const std::vector<std::string>& serverIndexVec, const std::vector<std::string>& locIndexVec) {
    std::string absolutePath = data._root + data._URIFilePath;

    std::vector<std::string>::const_iterator iter;
    std::string serverIndex;
    for (iter = serverIndexVec.begin(); iter != serverIndexVec.end(); ++iter) {
        if (FileController::checkType(absolutePath + *iter) == FileController::FILE) {
            serverIndex = *iter;
            break ;
        }
    }
    std::string locIndex;
    if (!locIndexVec.empty()) {
        for (iter = locIndexVec.begin(); iter != locIndexVec.end(); ++iter) {
            if (FileController::checkType(absolutePath + *iter) == FileController::FILE) {
                return (*iter);
                break ;
            }
        }
        return ("");
    } else {
        return (serverIndex);
    }
}

// NOTE - 설정해야 되는 부분: _errorPage, _errorPageList
// server block에 error_page 세팅이 있는 경우
// -- 있음: error_page 찾아서 에러 파일이 존재하는지 찾기
//         -- 존재: error_page로 사용
//         -- 존재안함: error_page는 ""
// -- 없음: error_page는 ""
// location block에 error_page 세팅이 있는 경우
// -- 있음: error_page 찾아서 에러 파일이 존재하는지 찾기
//         -- 존재: error_page로 사용
//         -- 존재안함: error_page는 ""
// -- 없음: serverBlock의  error_page가 있나?
//         -- 있음: server error_page 사용, _errorPageList는 server꺼로 사용
//         -- 없음: error_page는 "", _errorPageList도 ""
std::string HTTPResponseHandler::getErrorPage(const HTTPData& data, const std::vector<std::string>& serverErrorPageVec, const std::vector<std::string>& locErrorPageVec) {
    std::string absolutePath = data._root + data._URIFilePath;
    std::string serverErrorPage;
    std::vector<std::string>::const_iterator iter = serverErrorPageVec.end() - 1;
    if (!serverErrorPageVec.empty()) {
        if (FileController::checkType(absolutePath + *iter) == FileController::FILE) {
           _errorPageList  = serverErrorPageVec;
            serverErrorPage = *iter;
        } else {
            _errorPageList.clear(); 
            serverErrorPage = "";
        }
    } else {
        _errorPageList.clear(); 
        serverErrorPage = "";
    }
    std::string locErrorPage;
    iter = locErrorPageVec.end() - 1;
    if (!locErrorPageVec.empty()) {     // loc error page가 있는 경우
        if (FileController::checkType(absolutePath + *iter) == FileController::FILE) {
            _errorPageList = locErrorPageVec;
            locErrorPage = *iter;
        } else {
            _errorPageList.clear();
            locErrorPage = "";
        }
    } else {                            // loc error page가 없는 경우
        if (!serverErrorPage.empty()) { // server error page가 있는 경우
            _errorPageList = serverErrorPageVec;
            locErrorPage = serverErrorPage;
        } else {
            _errorPageList.clear();
            locErrorPage = "";
        }
    }
    return locErrorPage;
}

bool HTTPResponseHandler::isErrorPageList(int statusCode, std::vector<std::string>& errorPageVec) {
    std::stringstream ssStatusCode;
    ssStatusCode << statusCode;
    if (find(errorPageVec.begin(), errorPageVec.end(), ssStatusCode.str()) != errorPageVec.end()) {
        return true;
    }
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

void HTTPResponseHandler::setGeneralHeader(int status) {
    std::string startLine;
    if (status == 200) {
        startLine = std::string("HTTP/1.1 200 OK");
    } else if (status == 301) {
        startLine = std::string("HTTP/1.1 302 Found");
    } else if (status == 302) {
        startLine = std::string("HTTP/1.1 301 Moved Permanently");
    } else if (status == 400) {
        startLine = std::string("HTTP/1.1 400 Not Found");
    } else if (status == 403) {
        startLine = std::string("HTTP/1.1 403 Forbidden");
    } else if (status == 404) {
        startLine = std::string("HTTP/1.1 404 Not Found");
    } else if (status == 413) {
        startLine = std::string("HTTP/1.1 413 Payload Too Large");
    } else if (status == 500) {
        startLine = std::string("HTTP/1.1 500 Internal Server Error");
    } else {
        // FIXME 없는 경우 어떻게 처리할지 생각해보기
        startLine = "";        
    }

    time_t rawtime;
    std::time(&rawtime);
    struct tm* timeinfo = std::gmtime(&rawtime);
    static char timeBuffer[48];
    std::strftime(timeBuffer, 48, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

    _headerString = startLine;
    _headerString += "\r\n";
    _headers["Connection"] = std::string("close");
    _headers["Date"] = std::string(timeBuffer);
}

void HTTPResponseHandler::setHTMLHeader(const HTTPData& data) {
    std::stringstream ssLength;
    ssLength << data._resContentLength;
    _headers["Server"] = data._serverName;
    _headers["Content-Type"] = getMIME(data._URIExtension);
    _headers["Content-Length"] = ssLength.str();
    if (data._statusCode == 301) {
        _headers["Location"] = data._resAbsoluteFilePath;
    }
    convertHeaderMapToString();
}

HTTPResponseHandler::Phase HTTPResponseHandler::setError(HTTPData& data) {
    if (!_errorPage.empty() && isErrorPageList(data._statusCode, _errorPageList)) {
        data._statusCode = 200;
        setGeneralHeader(data._statusCode);
        data._resAbsoluteFilePath = data._root + _errorPage;
        data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
        return (GET_FILE);
    } else {
        setGeneralHeader(data._statusCode);
        data._URIExtension = "html";
        return (GET_STATIC_HTML);
    }
}

void HTTPResponseHandler::showResponseInformation(HTTPData &data) {
    std::cout << "Response Information=============================" << std::endl;
    std::cout << "# Request URL: " << data._reqURI << std::endl;
    std::cout << "# Status Code: " << data._statusCode << std::endl;
    std::cout << "# Absolute File Path: " << data._resAbsoluteFilePath << std::endl;
    std::cout << "# URL Extension: " << data._URIExtension << std::endl;
    std::cout << "# Location Index Page File: " << _indexPage << std::endl;
    std::cout << "# Location Error Page File: " << _errorPage << std::endl;
    std::cout << "# Location Path: " << _locConf.locationPath << std::endl;

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

HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data, long bufferSize) {
    // TODO: 해당하는 로케이션의 설정에서 status 코드 적용시키기
    if (data._statusCode == 400 || data._statusCode == 413) {
        // if (data._statusCode == 400) {
        //     std::cout << "[DEBUG] HTTP/1.1 400 error test" << std::endl;
        //     setGeneralHeader("HTTP/1.1 400 Not Found");
        //     data._URIExtension = "html";
        //     _phase = GET_STATIC_HTML;
        //}
        if (data._statusCode == 413) {
            std::cout << "[DEBUG] HTTP/1.1 413 error test" << std::endl;
            setGeneralHeader(data._statusCode);
            data._URIExtension = "html";
            _phase = GET_STATIC_HTML;
        }
    }
    if (_phase == FIND_RESOURCE) {
        // 1
        data._serverName = _serverConf.dirMap["server_name"];
        data._root = _serverConf.dirMap["root"];

        // 2
        for (std::size_t i = 0; i < _serverConf.location.size(); ++i) {
            std::string tmpExt = HTTPData::getExtension(_serverConf.location[i].locationPath);
            NginxConfig::LocationBlock tmpLocBlock = _serverConf.location[i];
            if (tmpExt.empty())
                continue ;
            _cgiConfMap[tmpExt] = tmpLocBlock.dirMap["cgi_pass"];
        }

        // 3
        bool isLocFlag = false;
        std::size_t matchLen = 0; 
        for (std::size_t i = 0; i < _serverConf.location.size(); ++i) {
            // NOTE: GUIDE LINE for Prefix Match
            // - request uri와 가장 일치하는 location 블록의 설정을 사용하되, 접근은 모두 request uri 기준으로하기!
            // 1. location path를 순환하며, path가 request uri의 앞부분에 포함되는지 찾기
            // 2. 만약 포함 된다면, 길이를 기억하기
            // 3. 가장 긴 길이(가장 알맞은 경로)의 path를 갖는 location block의 설정을 사용하기
            // 주의: 폴더/파일인지는 일단 상관 없음..! 여야하므로 항상 끝은 "/"로 끝나도록
            //      (ex. location /data/ab/ 인데, req uri: /data/a인 경우는 폴더가 아닌 파일)
            // 새로운 예외: location /data/ab만 있으면, req uri: /data/는 403 에러.. ==> 무조건, location path보다 req uri가 더 길어야한다..!
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
                    _locConf = _serverConf.location[i];
                }
            } else {
                if (matchLen < j && (data._URIFilePath.size() >= tmpLocPath.size())) { // 더 많은 글자가 일치하는 location 블록이 있는 경우
                    isLocFlag = true;
                    matchLen = j;
                    _locConf = _serverConf.location[i];
                }
            }
        }
        if (isLocFlag) { // 4
            // index page 세팅 및 error page 세팅
            _indexPage = getIndexPage(data, _serverConf.index, _locConf.index);
            _errorPage = getErrorPage(data, _serverConf.error_page, _locConf.error_page);
            _type = FileController::checkType(data._root + data._URIFilePath);
            // if (_type == FileController::DIRECTORY && data._URIFilePath[data._URIFilePath.size() - 1] == '/') {
            if (_type == FileController::DIRECTORY) {
                // TODO: 끝에 "/"가 붙어있는지 확인하기
                if (data._URIFilePath[data._URIFilePath.size() - 1] != '/') {
                    data._statusCode = 301;
                    setGeneralHeader(data._statusCode);
                    data._resAbsoluteFilePath = data._URIFilePath + "/";
                    data._URIExtension = "html";
                    _phase = GET_STATIC_HTML;
                } else {
                    if (!_locConf._return.empty()) { // redirection 시키는 것이 가장 우선순위가 높음
                        data._statusCode = atoi(_locConf._return[0].c_str());
                        setGeneralHeader(data._statusCode);
                        data._resAbsoluteFilePath = _locConf._return[1];
                        data._URIExtension = "html";
                        _phase = GET_STATIC_HTML;
                    } else { 
                        if (!_indexPage.empty()) { // index file이 어떻게든 있는 경우
                            // index 파일이 서버 컴퓨터에 있는지 판별
                            FileController::Type indexType = FileController::checkType(data._root + data._URIFilePath + _indexPage);
                            if (indexType == FileController::FILE) {
                                data._statusCode = 200;
                                setGeneralHeader(data._statusCode);
                                data._resAbsoluteFilePath = data._root + data._URIFilePath + _indexPage;
                                data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                                // 인덱스 파일이 cgi 파일인지 판별
                                if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
                                    data._CGIBinary = _cgiConfMap[data._URIExtension];
                                    _phase = CGI_RUN;
                                } else {
                                    setGeneralHeader("HTTP/1.1 200 OK");
                                    _phase = GET_FILE;
                                }
                            } else {
                                data._statusCode = 403;
                                _phase = setError(data);
                            }
                        } else {                  // index file이 어디에도 설정되지 않은 경우
                            _locConf.dirMap["autoindex"] = _locConf.dirMap["autoindex"].empty()
                                                        ? _serverConf.dirMap["autoindex"]
                                                        : _locConf.dirMap["autoindex"];
                            if (_locConf.dirMap["autoindex"] == "on") {
                                data._statusCode = 200;
                                setGeneralHeader(data._statusCode);
                                data._URIExtension = "html";
                                _phase = GET_STATIC_HTML;
                            } else {
                                data._statusCode = 403;
                                _phase = setError(data);
                            }
                        }
                    }
                }
            } else if (_type == FileController::FILE) {
                data._statusCode = 200;
                setGeneralHeader(data._statusCode);
                data._resAbsoluteFilePath = data._root + data._URIFilePath;
                data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
                    data._CGIBinary = _cgiConfMap[data._URIExtension];
                    _phase = CGI_RUN;
                } else {
                    setGeneralHeader("HTTP/1.1 200 OK");
                    _phase = GET_FILE;
                }
            } else {
                data._statusCode = 404;
                _phase = setError(data);
            }
        } else {
            if (!_errorPage.empty() && isErrorPageList(data._statusCode, _errorPageList)) {
                data._statusCode = 200;
                setGeneralHeader(data._statusCode);
                data._resAbsoluteFilePath = data._root + data._URIFilePath + _errorPage;
                data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
                _phase = GET_FILE;
            } else {
                data._statusCode = 404;
                _phase = setError(data);
            }
        }
        showResponseInformation(data);
    }
    if (_phase == GET_STATIC_HTML) {
        _staticHtml = HTMLBody::getStaticHTML(data);
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
        size_t writtenLengthOnBuf;
        size_t buflen = _staticHtml.empty() ? bufferSize : data._resContentLength + 1;
        Buffer buf(buflen);
        if (_staticHtml.empty()) {
            writtenLengthOnBuf = read(_file->getFd(), *buf, buflen);
            _phase = writtenLengthOnBuf == 0 ? FINISH : DATA_SEND_LOOP;
        } else {
            (*buf)[buflen - 1] = '\0';
            writtenLengthOnBuf = strlcpy(*buf, _staticHtml.c_str(), buflen);
            _phase = FINISH;
        }
        size_t writtenLengthOnSocket = send(_connectionFd, *buf, writtenLengthOnBuf, 0);
        if (writtenLengthOnSocket != writtenLengthOnBuf) {
            throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
        }
    } 

    if (_phase == CGI_RUN) {
        // FIXME[yekim]: _cgi를 처음에 절대 경로와 함께 생성하는데, 이를 그대로 사용하면 cgi가 동작하지 않습니다 ㅠㅠ
        delete _cgi;
        _cgi = new CGISession(data);
        _cgi->makeCGIProcess();
        //fcntl(_cgi->getOutputStream(), F_SETFL, O_NONBLOCK);
        _phase = CGI_REQ;
    }
    
    if (_phase == CGI_REQ) {
        if (data._postFilePath.empty()) {
            _phase = CGI_RECV_HEAD_LOOP;
        } else {
            _file = new FileController(data._postFilePath, FileController::READ);
            _phase = CGI_SEND_LOOP;
        }
    }
    
    if (_phase == CGI_SEND_LOOP) {
        Buffer buf(bufferSize);
        int length = read(_file->getFd(), *buf, bufferSize);
        if (length != 0) {
            int writeLength = write(_cgi->getInputStream(), *buf, length);
            if (writeLength != length) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            }
        } else {
            _file->del();
            _phase = CGI_RECV_HEAD_LOOP;
        }
    }
    
    if (_phase == CGI_RECV_HEAD_LOOP) {
        bool sendHeader = false;
        Buffer buf(bufferSize);
        int length = read(_cgi->getOutputStream(), *buf, bufferSize);
        std::cout << "length : " << length << std::endl;
        try { // 500 internal error 감지
            // NOTE: stdio는 라인바이라인으로 버퍼가 넘어가는데 여기서 eof(len = 0)가 오면 500 error임.
            if (length <= 0) {
                throw ErrorHandler("Error: CGI Internal Error", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            } else {
                _CGIReceive += std::string(*buf, length);
                size_t spliter = _CGIReceive.find("\r\n\r\n");
                std::string header;
                if (spliter != std::string::npos) {
                    header = _CGIReceive.substr(0, spliter);
                    _CGIReceive = _CGIReceive.substr(spliter + 4);
                    std::size_t pos = 0;
                    while (header.length() > pos) {
                        _headers.insert(getHTTPHeader(header, pos));
                    }
                    if (_headers.find("Status") == _headers.end()) {
                        data._statusCode = 200;
                        setGeneralHeader(data._statusCode);
                    } else {
                        // TODO[joopark]: setGeneralHeader 중복 세팅에 대한 부분 고려한 후 처리
                        data._statusCode = std::atoi(_headers["Status"].c_str());
                        setGeneralHeader(data._statusCode);
                        _headers.erase("Status");
                    }
                    sendHeader = true;
                }
            }
        } catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
            data._statusCode = 500;
            setGeneralHeader(data._statusCode);
            data._URIExtension = "html";
            _phase = GET_STATIC_HTML;
        }
        if (sendHeader == true) {
            convertHeaderMapToString();
            size_t writeLength = send(_connectionFd, _headerString.data(), _headerString.length(), 0);
            if (writeLength != _headerString.length()) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            }
            _phase = CGI_RECV_BODY_LOOP;
        }
    }
    
    if (_phase == CGI_RECV_BODY_LOOP) {
        std::cout << "CGI_RECV_BODY_LOOP" << std::endl;
        Buffer buf(bufferSize);
        if (_CGIReceive.empty()) {
            ssize_t length = read(_cgi->getOutputStream(), *buf, bufferSize);
            if (length == 0) {
                _phase = FINISH;
            } else if (length < 0) {
                throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            } else {
                ssize_t writeLength = send(_connectionFd, *buf, length, 0);
                if (writeLength != length) {
                    throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
                }
            }
        } else {
            size_t writeLength = send(_connectionFd, _CGIReceive.c_str(), _CGIReceive.length(), 0);
            if (writeLength != _CGIReceive.length()) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            } else {
                _CGIReceive.clear();
            }
        }
    }
    return (_phase);
}

int HTTPResponseHandler::getCGIfd(void) {
    return (_cgi->getOutputStream());
}