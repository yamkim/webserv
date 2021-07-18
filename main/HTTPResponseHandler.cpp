#include "HTTPResponseHandler.hpp"

// FIXME : 리스폰스에 URI 외에 더 다양한 아규먼트를 집어넣어야 하는데 어떤 형식으로 집어넣을지 고민 중입니다. 추후에 수정하겠습니다.
HTTPResponseHandler::HTTPResponseHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, const NginxConfig::NginxConfig& nginxConf) : HTTPHandler(connectionFd, serverConf, nginxConf) {
    _phase = PRE_STATUSCODE_CHECK;
    _file = NULL;
    _cgi = NULL;
}

HTTPResponseHandler::~HTTPResponseHandler() {
    delete _cgi;
    delete _file;
}


std::string HTTPResponseHandler::getIndexPage(const std::string& absPath) {
    std::vector<std::string>::const_iterator iter;
    std::string ret;


    if (!_locConf.index.empty()) {
        for (iter = _locConf.index.begin(); iter != _locConf.index.end(); ++iter) {
            if (FileController::checkType(absPath + *iter) == FileController::FILE) {
                ret = *iter;
                break ;
            }
        }
        if (iter == _locConf.index.end()) {
            ret = "";
        }
    } 
    return ret;
}

std::string HTTPResponseHandler::getErrorPage(const std::string& absPath) {
    std::vector<std::string>::const_iterator iter;

    std::string ret;
    if (!_locConf.error_page.empty()) {     // loc error page가 있는 경우
        iter = _locConf.error_page.end() - 1;
        if (FileController::checkType(absPath + *iter) == FileController::FILE) {
            _errorPageList = _locConf.error_page;
            ret = *iter;
        } else {
            _errorPageList.clear();
            ret = "";
        }
    } 
    return ret;
}


bool HTTPResponseHandler::isErrorPageList(int statusCode, std::vector<std::string>& errorPageVec) {
    std::stringstream ssStatusCode;
    ssStatusCode << statusCode;
    if (find(errorPageVec.begin(), errorPageVec.end(), ssStatusCode.str()) != errorPageVec.end()) {
        return true;
    }
    return false;
}

std::string HTTPResponseHandler::getMIME(const std::string& extension) {
    std::map<std::string, std::string> mime = _nginxConf._http.types.typeMap;
    if(extension == std::string("") || mime.find(extension) == mime.end()) {
        return (_nginxConf._http.dirMap["default_type"]);
    } else {
        return (mime[extension]);
    }
}

void HTTPResponseHandler::setGeneralHeader(HTTPData& data) {
    std::string startLine;
    std::stringstream ssStatusCode;
    ssStatusCode << data._statusCode;
    if (data._resStartLineMap.find(data._statusCode) != data._resStartLineMap.end()) {
        startLine = "HTTP/1.1 " + ssStatusCode.str() + " " + data._resStartLineMap[data._statusCode];
    } else {
        throw ErrorHandler("Error: invalid HTTP Status Code", ErrorHandler::ALERT, "HTTPResponseHandler::setGeneralHeader");   
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
    _headers["Server"] = std::string("webserv/") + std::string(WEBSERV_VERSION);
    _headers["Content-Type"] = getMIME(data._URIExtension);
    _headers["Content-Length"] = ssLength.str();
    if (data._statusCode == 301) {
        _headers["Location"] = data._resAbsoluteFilePath;
    }
    convertHeaderMapToString();
}

void HTTPResponseHandler::showResponseInformation(HTTPData &data) {
    std::cout << "Response Information=============================" << std::endl;
    std::cout << "# Typed Request URL by client: https://127.0.0.1:4242" << data._reqURI << std::endl;
    std::cout << "# root + Request URL: " << data._root + data._reqURI << std::endl;
    std::cout << "# Request Method: " << data._reqMethod << std::endl;
    std::cout << "# Request URL: " << data._reqURI << std::endl;
    std::cout << "# Status Code: " << data._statusCode << std::endl;
    std::cout << "# Absolute File Path: " << data._resAbsoluteFilePath << std::endl;
    std::cout << "# URL Extension: " << data._URIExtension << std::endl;
    std::cout << "# Location Index Page File: " << _indexPage << std::endl;
    std::cout << "# Location Error Page File: " << _errorPage << std::endl;
    std::cout << "# Location Path: " << _locConf._locationPath << std::endl;

}

void HTTPResponseHandler::setCGIConfigMap() {
    for (std::size_t i = 0; i < _serverConf.location.size(); ++i) {
        std::string tmpExt = HTTPData::getExtension(_serverConf.location[i]._locationPath);
        NginxConfig::LocationBlock tmpLocBlock = _serverConf.location[i];
        if (tmpExt.empty()) {
            continue ;
        }
        _cgiConfMap[tmpExt] = tmpLocBlock.dirMap["cgi_pass"];
    }
}

NginxConfig::LocationBlock HTTPResponseHandler::getMatchingLocationConfiguration(const HTTPData& data) {
    NginxConfig::LocationBlock ret;
    std::string URI = data._URIFilePath;
    int position = -1;
    std::size_t maxLength = 0;
    for (std::size_t i = 0; i < _serverConf.location.size(); ++i) {
        std::string target = _serverConf.location[i]._locationPath;
        if (URI.compare(0, target.length(), target) == 0) {
            if (maxLength <= target.length()) {
                position = i;
            }
        }
    }
    if (position != -1) {
        return (_serverConf.location[position]);
    } else {
        return (ret);
    }
}

HTTPResponseHandler::Phase HTTPResponseHandler::setInformation(HTTPData& data, int statusCode, const std::string& absPath) {
    data._statusCode = statusCode;
    setGeneralHeader(data);
    data._resAbsoluteFilePath = absPath;
    data._URIExtension = "html";
    if (data._statusCode == 200) {
        data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
        // 인덱스 파일이 cgi 파일인지 판별
        if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
            data._CGIBinary = _cgiConfMap[data._URIExtension];
            return CGI_RUN;
        } else {
            return GET_FILE;
        }
    }
    if (!_errorPage.empty() && isErrorPageList(data._statusCode, _errorPageList)) {
        return setInformation(data, 200, data._resAbsoluteFilePath + _errorPage);
    }
    return GET_STATIC_HTML;
}

HTTPResponseHandler::Phase HTTPResponseHandler::setFileInDirectory(HTTPData& data, const std::string& absLocPath) {
    if (!_locConf.allowed_method.empty() 
        && find(_locConf.allowed_method.begin(), _locConf.allowed_method.end(), data._reqMethod) == _locConf.allowed_method.end()) {
        return setInformation(data, 405, data._root + "/");
    } else {
        if (!_indexPage.empty()) { // index file이 어떻게든 있는 경우
            // index 파일이 서버 컴퓨터에 있는지 판별
            FileController::Type indexType = FileController::checkType(absLocPath + _indexPage);
            if (indexType == FileController::FILE) {
                _phase = setInformation(data, 200, absLocPath + _indexPage);
            } else {
                // _phase = setInformation(data, 403, absLocPath);
                _phase = setInformation(data, 404, absLocPath);
            }
        } else {                  // index file이 어디에도 설정되지 않은 경우
            _locConf.dirMap["autoindex"] = _locConf.dirMap["autoindex"].empty()
                                        ? _serverConf.dirMap["autoindex"]
                                        : _locConf.dirMap["autoindex"];
            if (_locConf.dirMap["autoindex"] == "on") {
                _phase = setInformation(data, 200, "");
                data._statusCode = 200;
                setGeneralHeader(data);
                data._URIExtension = "html";
                _phase = GET_STATIC_HTML;
            } else {
                _phase = setInformation(data, 403, absLocPath);
            }
        }
    }
    return _phase;
}


HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data, long bufferSize) {
    // TODO: 해당하는 로케이션의 설정에서 status 코드 적용시키기
    if (_phase == PRE_STATUSCODE_CHECK) {
        if (data._statusCode != 200) {
            setGeneralHeader(data);
            data._URIExtension = "html";
            _phase = GET_STATIC_HTML;
        } else {
            _phase = FIND_RESOURCE; 
        }
    }
    
    if (_phase == FIND_RESOURCE) {
        // 1
        data._serverName = _serverConf.dirMap["server_name"];

        // 2
        setCGIConfigMap();

        // 3
        _locConf = getMatchingLocationConfiguration(data);
        
        // TODO: root에 따라서 변하는 경우, 처리할지 말지 고민: location block 내에도 root가 올 수 있음
        if (!_locConf._locationPath.empty()) { // 4
            // index page 세팅 및 error page 세팅
            // http://localhost:4242/static_html/index.html

            // data.URIFilePath: /static_html/index.html
            // tmpFile: /static_html/index.html
            // tmpLocPath: /static_html/ (location path가 정의되었다면)
            // absLocPath: root/static_html/
            // absFilePath: root/static_html/index.html
            data._root = _locConf.dirMap["root"];
            std::string tmpFilePath = data._URIFilePath;
            std::string tmpLocPath = _locConf._locationPath == "/" ? "/" : _locConf._locationPath + "/";
            std::string absFilePath = data._root + tmpFilePath;

            _indexPage = getIndexPage(data._root + tmpLocPath);
            _errorPage = getErrorPage(data._root + tmpLocPath);
            if ((_locConf.inner_proxy.size() != 0) && (data._originURI == data._reqURI)) {
                data._reqURI = _locConf.inner_proxy[0];
                data.setURIelements();
                int clientMaxBodySize = _locConf.dirMap["client_max_body_size"].empty() ? -1 : std::atoi(_locConf.dirMap["client_max_body_size"].c_str());
                if (clientMaxBodySize >= 0) {
                    if (clientMaxBodySize < std::atoi(data._reqContentLength.c_str())) {
                        data._statusCode = 413;
                    }
                }
                _phase = PRE_STATUSCODE_CHECK;
            } else {
                _type = FileController::checkType(absFilePath);
                if (_type == FileController::DIRECTORY) {
                    if (data._URIFilePath[data._URIFilePath.size() - 1] != '/') {
                        _phase = setInformation(data, 301, data._URIFilePath + "/");
                    } else {
                        if (!_locConf._return.empty()) { // redirection 시키는 것이 가장 우선순위가 높음
                            _phase = setInformation(data, atoi(_locConf._return[0].c_str()), _locConf._return[1]);
                        } else {
                            _phase = setFileInDirectory(data, data._root + tmpLocPath);
                        }
                    }
                } else if (_type == FileController::FILE) {
                    _phase = setInformation(data, 200, absFilePath);
                } else {
                    tmpFilePath = tmpFilePath.substr(_locConf._locationPath.size());
                    absFilePath = data._root + tmpFilePath;

                    _indexPage = getIndexPage(data._root + "/");
                    _errorPage = getErrorPage(data._root + "/");
                    _type = FileController::checkType(absFilePath);
                    if (_type == FileController::DIRECTORY) {
                        if (data._URIFilePath[data._URIFilePath.size() - 1] != '/') {
                            _phase = setInformation(data, 301, data._URIFilePath + "/");
                        } else {
                            if (!_locConf._return.empty()) { // redirection 시키는 것이 가장 우선순위가 높음
                                _phase = setInformation(data, atoi(_locConf._return[0].c_str()), _locConf._return[1]);
                            } else {
                                _phase = setFileInDirectory(data, absFilePath);
                            }
                        }
                    } else if (_type == FileController::FILE) {
                        _phase = setInformation(data, 200, absFilePath);
                    } else {
                        _phase = setInformation(data, 404, data._root + "/");
                    }
                }
            }
        } else { // TODO: locatioin에 대한 정보가 없는 경우 어떻게 처리할건지 고려 => root가 있으면 root에서 index 파일을 찾아야됨.
            _phase = setInformation(data, 404, data._root + _locConf._locationPath + "/");
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
        _cgi = new CGISession(data);
        if (data._postFilePath.empty()) {
            _cgi->makeCGIProcess(0);
         }else {
            _file = new FileController(data._postFilePath, FileController::READ);
            _cgi->makeCGIProcess(_file->getFd());
        }
        _phase = CGI_RECV_HEAD_LOOP;
    }
    
    if (_phase == CGI_RECV_HEAD_LOOP) {
        bool sendHeader = false;
        Buffer buf(bufferSize);
        int length = read(_cgi->getOutputStream(), *buf, bufferSize);
        std::cout << "length : " << length << std::endl;
        try { // 500 internal error 감지
            // NOTE: stdio는 라인바이라인으로 버퍼가 넘어가는데 여기서 eof(len = 0)가 오면 500 error임.
            if (length <= 0) {
                throw ErrorHandler("Error: CGI HTTP Header Error", ErrorHandler::NORMAL, "HTTPResponseHandler::process");
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
                        setGeneralHeader(data);
                    } else {
                        // TODO[joopark]: setGeneralHeader 중복 세팅에 대한 부분 고려한 후 처리
                        data._statusCode = std::atoi(_headers["Status"].c_str());
                        setGeneralHeader(data);
                        _headers.erase("Status");
                    }
                    sendHeader = true;
                }
            }
        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
            data._statusCode = 500;
            setGeneralHeader(data);
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