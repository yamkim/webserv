#include "HTTPRequestHandler.hpp"

HTTPRequestHandler::HTTPRequestHandler(int connectionFd, const NginxConfig::ServerBlock& serverConf, NginxConfig::NginxConfig& nginxConf) : HTTPHandler(connectionFd, serverConf, nginxConf) {
    _phase = PARSE_STARTLINE;
    _contentLength = -1;
    _contentLengthSum = 0;
    _chunkFinish = false;
    _dynamicBufferSize = 0;
    _fileController = NULL;
    _stringBuffer = std::string("");
    _stringBufferClear = false;
}

HTTPRequestHandler::~HTTPRequestHandler() {
    delete _fileController;
}

HTTPRequestHandler::Phase HTTPRequestHandler::process(HTTPData& data, long bufferSize) {
    _dynamicBufferSize = bufferSize;
    if (_phase == PARSE_STARTLINE) {
        data._statusCode = 200;
        if (getStartLine(data) == true) {
            _phase = PARSE_HEADER;
        }
    } else if (_phase == PARSE_HEADER) {
        if (getHeader() == true) {
            data.setHTTPCGIENV(_headers);
            if (_headers.find("Transfer-Encoding") != _headers.end()) {
                if (_headers["Transfer-Encoding"].find("chunked") == std::string::npos) {
                    throw ErrorHandler("Error: Transfer-Encoding header error.", ErrorHandler::NORMAL, "HTTPRequestHandler::process");
                } else {
                    data._postFilePath = Utils::randomStringGenerator(20);
                    _fileController = new FileController(data._postFilePath, FileController::WRITE);
                    _phase = PARSE_BODY_CHUNK;
                }
            } else if (_headers.find("Content-Length") != _headers.end()) {
                _contentLength = std::atoi(_headers["Content-Length"].c_str());
                if (_contentLength > std::atoi(_serverConf.dirMap["client_max_body_size"].c_str())) {
                    data._statusCode = 413;
                    _phase = FINISH;
                } else {
                    data._postFilePath = Utils::randomStringGenerator(20);
                    _fileController = new FileController(data._postFilePath, FileController::WRITE);
                    _phase = PARSE_BODY_NBYTES;
                }
            } else {
                _phase = FINISH;
            }
        }
    } else if (_phase == PARSE_BODY_CHUNK) {
        std::string* bufptr = getDataByCRNF(10);
        if (bufptr != NULL) {
            _contentLength = Utils::hextoint(bufptr->c_str());
            if (_contentLength >= 0) {
                if (_contentLength == 0) {
                    _chunkFinish = true;
                }
                _contentLengthSum += _contentLength;
                _phase = PARSE_BODY_NBYTES;
            } else {
                throw ErrorHandler("Error: chunk length error.", ErrorHandler::NORMAL, "HTTPRequestHandler::process");
            }
        }
    } else if (_phase == PARSE_BODY_NBYTES) {
        Buffer buffer(_dynamicBufferSize);
        ssize_t readBufLength = (_contentLength >= _dynamicBufferSize) ? _dynamicBufferSize : _contentLength;
        int readLength = recv(_connectionFd, *buffer, readBufLength, 0);
        //std::cout << "readLength : " << readLength << " / " << _contentLengthSum << std::endl;
        if (readLength == -1) {
            throw ErrorHandler("Error: socket read (request body) error.", ErrorHandler::ALERT, "HTTPRequestHandler::process");
        }
        _contentLength -= readLength;
        readLength = write(_fileController->getFd(), *buffer, readLength);
        if (readLength == -1) {
            throw ErrorHandler("Error: file write (request body) error.", ErrorHandler::ALERT, "HTTPRequestHandler::process");
        }
        if (_contentLength == 0) {
            if (_headers.find("Content-Length") != _headers.end()) {
                data._reqContentType = _headers["Content-Type"];
                data._reqContentLength = _headers["Content-Length"];
                _phase = FINISH;
            } else {
                _phase = REMOVE_CRNF;
            }
        }
    } else if (_phase == REMOVE_CRNF) {
        std::string* bufptr = getDataByCRNF(10);
        if (bufptr != NULL) {
            if (_chunkFinish) {
                data._reqContentType = std::string("application/octet-stream");
                data._reqContentLength = Utils::ltos(_contentLengthSum);
                _phase = FINISH;
            } else {
                _phase = PARSE_BODY_CHUNK;
            }
        }
    }
    return (_phase);
}

bool HTTPRequestHandler::getStartLine(HTTPData& data) {
    std::string* bufptr = getDataByCRNF(1000);
    if (bufptr == NULL) {
        return (false);
    }
    std::vector<std::string> tmp = Parser::getSplitBySpace(*bufptr);
    if (tmp.size() != 3) {
        throw ErrorHandler("Error: invalid HTTP Header.", ErrorHandler::NORMAL, "HTTPRequestHandler::getStartLine");
    }
    if (   tmp[0] == std::string("GET")
        || tmp[0] == std::string("HEAD")
        || tmp[0] == std::string("POST")
        || tmp[0] == std::string("PUT")
        || tmp[0] == std::string("DELETE")
        || tmp[2] != std::string("HTTP/1.1")) {
        data._reqMethod = tmp[0];
        data._reqURI = tmp[1];
        data._originURI = tmp[1];
        data.setURIelements();
    } else {
        throw ErrorHandler("Error: invalid HTTP Header.", ErrorHandler::NORMAL, "HTTPRequestHandler::process");
    }
    return (true);
}

bool HTTPRequestHandler::getHeader(void) {
    std::string* bufptr = getDataByCRNF(1000);
    if (bufptr == NULL) {
        return (false);
    }
    if (bufptr->length() == 0) {
        return (true);
    }
    std::size_t pos = 0;
    // FIXME: 수정 검토
    *bufptr = *bufptr + std::string("\r\n");
    _headers.insert(getHTTPHeader(*bufptr, pos));
    return (false);
}

std::string* HTTPRequestHandler::getDataByCRNF(int searchLength) {
    long dynamicBufferSize;
    if (searchLength > 0) {
        dynamicBufferSize = (_dynamicBufferSize > searchLength) ? searchLength : _dynamicBufferSize;
    } else {
        dynamicBufferSize = _dynamicBufferSize;
    }
    Buffer buffer(dynamicBufferSize);
    if (_stringBufferClear == true) {
        _stringBuffer.clear();
        _stringBufferClear = false;
    }
    ssize_t peekLength = recv(_connectionFd, *buffer, dynamicBufferSize, MSG_PEEK);
    if (peekLength == TRANS_ERROR) {
        throw ErrorHandler("Error: recv error.", ErrorHandler::ALERT, "HTTPRequestHandler::getDataByCRNF");
    }
    size_t bufferLength = _stringBuffer.length();
    _stringBuffer += std::string(*buffer, peekLength);
    size_t CRNFPosition = _stringBuffer.find("\r\n");
    if (CRNFPosition == std::string::npos) {
        ssize_t readLength = recv(_connectionFd, *buffer, peekLength, 0);
        if (readLength == TRANS_ERROR) {
            throw ErrorHandler("Error: recv error.", ErrorHandler::ALERT, "HTTPRequestHandler::getDataByCRNF");
        }
        return (NULL);
    } else {
        peekLength = (CRNFPosition + 2) - bufferLength;
        ssize_t readLength = recv(_connectionFd, *buffer, peekLength, 0);
        if (readLength == TRANS_ERROR) {
            throw ErrorHandler("Error: recv error.", ErrorHandler::ALERT, "HTTPRequestHandler::getDataByCRNF");
        }
        _stringBuffer = _stringBuffer.substr(0, CRNFPosition);
        _stringBufferClear = true;
        return (&_stringBuffer);
    }
}
