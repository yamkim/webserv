#include "HTTPResponseHandler.hpp"

HTTPResponseHandler::HTTPResponseHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, const NginxConfig::GlobalConfig& nginxConf) : HTTPHandler(connectionFd, serverConf, nginxConf) {
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
    if (!_locConf.error_page.empty()) {
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
    if (data.getResStartLineMap(data._statusCode).empty() == false) {
        startLine = "HTTP/1.1 " + ssStatusCode.str() + " " + data.getResStartLineMap(data._statusCode);
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
    if ( (data._HTTPCGIENV.find("HTTP_CONNECTION") != data._HTTPCGIENV.end())
        && (data._HTTPCGIENV["HTTP_CONNECTION"] == std::string("close"))) {
        _headers["Connection"] = std::string("close");
    } else {
        _headers["Connection"] = std::string("keep-alive");
    }
}

void HTTPResponseHandler::setHTMLHeader(const HTTPData& data) {
    std::stringstream ssLength;
    ssLength << data._resContentLength;
    _headers["Server"] = std::string("webserv/") + std::string(WEBSERV_VERSION);
    _headers["Content-Type"] = getMIME(data._URIExtension);
    _headers["Content-Length"] = ssLength.str();
    if (data._statusCode == 301 || data._statusCode == 302
        || data._statusCode == 307 || data._statusCode == 308) {
        _headers["Location"] = data._resAbsoluteFilePath;
    }
    convertHeaderMapToString();
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
    int clientMaxBodySize = _locConf.dirMap["client_max_body_size"].empty() ? -1 : std::atol(_locConf.dirMap["client_max_body_size"].c_str());
    if (!_locConf.allowed_method.empty() 
        && find(_locConf.allowed_method.begin(), _locConf.allowed_method.end(), data._reqMethod) == _locConf.allowed_method.end()) {
        data._statusCode = 405;
        setGeneralHeader(data);
        data._resAbsoluteFilePath = absPath;
        data._URIExtension = "html";
    } else if (clientMaxBodySize >= 0 && clientMaxBodySize < std::atol(data._reqContentLength.c_str())) {
        data._statusCode = 413;
        return (PRE_STATUSCODE_CHECK);
    } else {
        data._statusCode = statusCode;
        setGeneralHeader(data);
        data._resAbsoluteFilePath = absPath;
        data._URIExtension = "html";
        if (data._statusCode == 200) {
            data._URIExtension = HTTPData::getExtension(data._resAbsoluteFilePath);
            if (_cgiConfMap.find(data._URIExtension) != _cgiConfMap.end()) {
                data._CGIBinary = _cgiConfMap[data._URIExtension];
                return CGI_RUN;
            } else {
                return GET_FILE;
            }
        }
    }
    if (!_errorPage.empty() && isErrorPageList(data._statusCode, _errorPageList)) {
        return setInformation(data, 200, data._resAbsoluteFilePath + _errorPage);
    }
    return GET_STATIC_HTML;
}

HTTPResponseHandler::Phase HTTPResponseHandler::setFileInDirectory(HTTPData& data, const std::string& absLocPath) {
    int clientMaxBodySize = _locConf.dirMap["client_max_body_size"].empty() ? -1 : std::atol(_locConf.dirMap["client_max_body_size"].c_str());

    if (!_locConf.allowed_method.empty() 
        && find(_locConf.allowed_method.begin(), _locConf.allowed_method.end(), data._reqMethod) == _locConf.allowed_method.end()) {
        return setInformation(data, 405, data._root + "/");
    } else if (clientMaxBodySize >= 0 && clientMaxBodySize < std::atol(data._reqContentLength.c_str())) {
        data._statusCode = 413;
        return (PRE_STATUSCODE_CHECK);
    } else {
        if (!_indexPage.empty()) {
            FileController::Type indexType = FileController::checkType(absLocPath + _indexPage);
            if (indexType == FileController::FILE) {
                _phase = setInformation(data, 200, absLocPath + _indexPage);
            } else {
                _phase = setInformation(data, 404, absLocPath);
            }
        } else {
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

HTTPResponseHandler::Phase HTTPResponseHandler::handleProcess(std::string tmpFilePath, std::string tmpLocPath, std::string absFilePath, HTTPData& data) {
    _indexPage = getIndexPage(data._root + tmpLocPath);
    _errorPage = getErrorPage(data._root + tmpLocPath);
    _type = FileController::checkType(absFilePath);
    if (_type == FileController::DIRECTORY) {
        if (data._URIFilePath[data._URIFilePath.size() - 1] != '/') {
            _phase = setInformation(data, 301, data._URIFilePath + "/");
        } else {
            if (!_locConf._return.empty()) {
                _phase = setInformation(data, atol(_locConf._return[0].c_str()), _locConf._return[1]);
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
                if (data.getMethod() == std::string("POST")) {
                    _phase = setInformation(data, 308, data._URIFilePath + "/");
                } else {
                    _phase = setInformation(data, 301, data._URIFilePath + "/");
                }
            } else {
                if (!_locConf._return.empty()) {
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
    return _phase;
}


HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data, long bufferSize) {
    if (_phase == PRE_STATUSCODE_CHECK) {
        if (data._statusCode != 200) {
            setGeneralHeader(data);
            data._URIExtension = "html";
            _phase = GET_STATIC_HTML;
        } else {
            _phase = FIND_RESOURCE; 
        }
    } else if (_phase == FIND_RESOURCE) {
        data._serverName = _serverConf.dirMap["server_name"];
        setCGIConfigMap();
        _locConf = getMatchingLocationConfiguration(data);
        if (!_locConf._locationPath.empty()) {
            data._root = _locConf.dirMap["root"];

            if ((_locConf.inner_proxy.size() != 0) && (data._originURI == data._reqURI)) {
                data._reqURI = _locConf.inner_proxy[0];
                data.setURIelements();


                std::string tmpFilePath = data._URIFilePath;
                std::string tmpLocPath = "/";
                std::string absFilePath = data._root + tmpFilePath;
                _phase = handleProcess(tmpFilePath, tmpLocPath, absFilePath, data);
            } else {
                std::string tmpFilePath = data._URIFilePath;
                std::string tmpLocPath = _locConf._locationPath == "/" ? "/" : _locConf._locationPath + "/";
                std::string absFilePath = data._root + tmpFilePath;
                _phase = handleProcess(tmpFilePath, tmpLocPath, absFilePath, data);
            }
        } else { 
            _phase = setInformation(data, 404, data._root + _locConf._locationPath + "/");
        }
    }
    
    else if (_phase == GET_STATIC_HTML) {
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
    } else if (_phase == DATA_SEND_LOOP) {
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
            throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process @ GET_STATIC_HTML");
        }
    } else if (_phase == CGI_RUN) {
        _cgi = new CGISession(data);
        if (data._postFilePath.empty()) {
            _cgi->makeCGIProcess(0);
         }else {
            _file = new FileController(data._postFilePath, FileController::READ);
            _cgi->makeCGIProcess(_file->getFd());
        }
        _phase = CGI_RECV_HEAD_LOOP;
    } else if (_phase == CGI_RECV_HEAD_LOOP) {
        bool sendHeader = false;
        Buffer buf(bufferSize);
        int length = read(_cgi->getOutputStream(), *buf, bufferSize);
        try {
            if (length <= 0) {
                throw ErrorHandler("Error: CGI HTTP Header Error", ErrorHandler::NORMAL, "HTTPResponseHandler::process @ CGI_RECV_HEAD_LOOP");
            } else {
                _CGIReceive += std::string(*buf, length);
                size_t spliter = _CGIReceive.find("\r\n\r\n");
                std::string header;
                if (spliter != std::string::npos) {
                    header = _CGIReceive.substr(0, spliter);
                    _CGIReceive = _CGIReceive.substr(spliter + 4);
                    std::size_t pos = 0;
                    long multipleHeaderCount = 0;
                    while (header.length() > pos) {
                        std::pair<std::string, std::string> tmp = getHTTPHeader(header, pos);
                        if (tmp.first == std::string("Set-Cookie")) {
                            tmp.first = Utils::ltos(multipleHeaderCount++) + std::string("@") + tmp.first;
                        }
                        _headers.insert(tmp);
                    }
                    _headers["Connection"] = std::string("close");
                    if (_headers.find("Status") == _headers.end()) {
                        data._statusCode = 200;
                        setGeneralHeader(data);
                    } else {
                        data._statusCode = std::atoi(_headers["Status"].c_str());
                        setGeneralHeader(data);
                        _headers.erase("Status");
                    }
                    sendHeader = true;
                }
            }
        } catch (const std::exception& error) {
            std::cerr << "[HTTPResponseHandler::process] " << error.what() << std::endl;
            data._statusCode = 500;
            setGeneralHeader(data);
            data._URIExtension = "html";
            _phase = GET_STATIC_HTML;
        }
        if (sendHeader == true) {
            convertHeaderMapToString();
            size_t writeLength = send(_connectionFd, _headerString.data(), _headerString.length(), 0);
            if (writeLength != _headerString.length()) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process @ CGI_RECV_HEAD_LOOP");
            }
            _phase = CGI_RECV_BODY_LOOP;
        }
    } else if (_phase == CGI_RECV_BODY_LOOP) {
        Buffer buf(bufferSize);
        if (_CGIReceive.empty()) {
            ssize_t length = read(_cgi->getOutputStream(), *buf, bufferSize);
            if (length == 0) {
                _phase = FINISH;
            } else if (length < 0) {
                throw ErrorHandler("Error: read error.", ErrorHandler::ALERT, "HTTPResponseHandler::process @ CGI_RECV_BODY_LOOP");
            } else {
                ssize_t writeLength = send(_connectionFd, *buf, length, 0);
                if (writeLength != length) {
                    throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process @ CGI_RECV_BODY_LOOP");
                }
            }
        } else {
            size_t writeLength = send(_connectionFd, _CGIReceive.c_str(), _CGIReceive.length(), 0);
            if (writeLength != _CGIReceive.length()) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process @ CGI_RECV_BODY_LOOP");
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