#include "HTTPResponseHandler.hpp"

// FIXME : 리스폰스에 URI 외에 더 다양한 아규먼트를 집어넣어야 하는데 어떤 형식으로 집어넣을지 고민 중입니다. 추후에 수정하겠습니다.
HTTPResponseHandler::HTTPResponseHandler(int connectionFd) {
    _connectionFd = new int(0);
    _file = new FileController*;
    _type = new FileController::Type;
    _nginxConfig = new NginxConfig("nginx.conf");
    _phase = new Phase(FIND_RESOURCE);
    // FIXME : root 경로와 같은 정보는 .conf 파일에서 받아와야 합니다.
    _root = new std::string();
    _serverIndex = new std::string();
    _staticHtml = new std::string();
    _absolutePath = new std::string();
    *_root = (*_nginxConfig)._http.server[1].dirMap["root"];

    // FIXME: 일단 공통 구조체를 process 내에서만 사용해서... 이 변수들에 대해 조치가 필요합니다.
    (*_serverIndex) = getServerIndex((*_nginxConfig)._http.server[1]);

    _cgi = NULL;
	(*_connectionFd) = connectionFd;
	_headerString = new std::string("");
    _headers = new std::map<std::string, std::string>;
}

HTTPResponseHandler::~HTTPResponseHandler() {
    delete _cgi;
    delete _file;
}

std::string HTTPResponseHandler::getServerIndex(NginxConfig::ServerBlock server) {
    // std::string indeces = server.index + ";";
    // std::size_t pos = 0;
    // std::string tmpServerIndex[3];
    // tmpServerIndex[0] = NginxParser::getIdentifier(indeces, pos, " ");
    // tmpServerIndex[1] = NginxParser::getIdentifier(indeces, pos, ";");
    for (int i = 0; i < (int)server.index.size(); i++) {
        if (FileController::checkType((*_absolutePath) + server.index[i]) == FileController::FILE) {
            return (server.index[i]);
        }
    }
    return (std::string(""));
}

void HTTPResponseHandler::responseNotFound(const HTTPData& data) {
    std::string notFoundType = std::string("html");
    (*_staticHtml) = HTMLBody::getStaticHTML(data._statusCode);
    setHTMLHeader("html", (*_staticHtml).length());
    send((*_connectionFd), (*_headerString).data(), (*_headerString).length(), 0);
    (*_phase) = DATA_SEND_LOOP;
}

void HTTPResponseHandler::responseAutoIndex(const HTTPData& data) {
    std::string autoIndexType = std::string("html");
    (*_staticHtml) = HTMLBody::getAutoIndexBody((*_root), data._URIFilePath);
    setHTMLHeader("html", (*_staticHtml).length());
    send((*_connectionFd), (*_headerString).data(), (*_headerString).length(), 0);
    (*_phase) = DATA_SEND_LOOP;
}

void HTTPResponseHandler::setHTMLHeader(const std::string& extension, const long contentLength) {
    setTypeHeader(getMIME(extension));
    setLengthHeader(contentLength);
    convertHeaderMapToString(false);
}


void HTTPResponseHandler::setGeneralHeader(std::string status) {
    static char timeBuffer[48];
    time_t rawtime;

    std::time(&rawtime);
    struct tm* timeinfo = std::gmtime(&rawtime);
    std::strftime(timeBuffer, 48, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

    (*_headerString) = status;
    (*_headerString) += "\r\n";

    // NOTE https://developer.mozilla.org/ko/docs/Web/HTTP/Headers
    // 서버의 소프트웨어 정보
    (*_headers)["Server"] = std::string("webserv/0.1");
    // HTTP 메시지가 만들어진 날짜와 시간
    (*_headers)["Date"] = std::string(timeBuffer);
    // 전송이 완료된 후 네트워크 접속을 유지할지 말지
    // REVIEW 상황따라 keep-alive / close가 갈리긴 하지만 현재 구현상으로는 메시지 보내고 끊김. keep-alive 형태로 현재 연결된 fd를 유지시켜야 하나?
    (*_headers)["Connection"] = std::string("close");
}

void HTTPResponseHandler::setTypeHeader(std::string type) {
    (*_headers)["Content-Type"] = type;
}

void HTTPResponseHandler::setLengthHeader(long contentLength) {
    std::stringstream ssLength;

    ssLength << contentLength;
    (*_headers)["Content-Length"] = ssLength.str();
}

void HTTPResponseHandler::convertHeaderMapToString(bool isCGI) {
    std::map<std::string, std::string>::iterator iter;
    for (iter = (*_headers).begin(); iter != (*_headers).end(); ++iter) {
        (*_headerString) += iter->first;
        (*_headerString) += ": ";
        (*_headerString) += iter->second;
        (*_headerString) += "\r\n";
    }
    if (isCGI == false) {
        (*_headerString) += "\r\n";
    }
}

HTTPResponseHandler::Phase HTTPResponseHandler::process(HTTPData& data) {
    if (*_phase == FIND_RESOURCE) {
        // TODO: data 인수에 request 파싱된 결과가 들어있어서 이 클래스 초기화될때 data를 넣어서 초기화 하거나 여기서 초기화 해야합니다.
        (*_absolutePath) = (*_root) + data._URIFilePath;
        (*_type) = FileController::checkType((*_absolutePath));
        data._requestAbsoluteFilePath = (*_absolutePath) + (*_serverIndex);
        if ((*_type) == FileController::NOTFOUND) {
            setGeneralHeader("HTTP/1.1 404 Not Found");
            *_phase = NOT_FOUND;
        } else if ((*_type) == FileController::DIRECTORY) {
            setGeneralHeader("HTTP/1.1 200 OK");
            if ((*_serverIndex).empty()) { // 만약 _serverIndex가 없다면 바로 auto index로 띄우기
                *_phase = AUTOINDEX;
            } else {                    // 만약 dierctory이고 server index가 있다면 절대 경로에 index 더하기
                (*_absolutePath) = (*_absolutePath) + (*_serverIndex);
                *_phase = GET_FILE;
            }
        } else if ((*_type) == FileController::FILE) {
            setGeneralHeader("HTTP/1.1 200 OK");
            if (isCGI(data._URIExtension)) {
                *_phase = CGI_RUN;
            } else {
                *_phase = GET_FILE;
            }
        }
    } 

    if (*_phase == NOT_FOUND) {
        responseNotFound(data);
    }

    if (*_phase == AUTOINDEX) {
        responseAutoIndex(data);
    }
    
    if (*_phase == GET_FILE) {
        (*_file) = new FileController((*_absolutePath), FileController::READ);
        setHTMLHeader(data._URIExtension, (*_file)->length());
        send((*_connectionFd), (*_headerString).data(), (*_headerString).length(), 0);
        *_phase = DATA_SEND_LOOP;
    }
    
    #if 1 // HTML Part
    if (*_phase == DATA_SEND_LOOP) {
        // FIXME: 조금 더 이쁘장하게 수정해야 할 듯 합니다...
        if ((*_staticHtml).empty() == false) {
            size_t writeLength = send((*_connectionFd), (*_staticHtml).c_str(), (*_staticHtml).length(), 0);
            if (writeLength != (*_staticHtml).length()) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            } else {
                *_phase = FINISH;
            }
        } else {
            char buf[RESPONSE_BUFFER_SIZE];
            int length = read((*_file)->getFd(), buf, RESPONSE_BUFFER_SIZE);
            if (length != 0) {
                int writeLength = send((*_connectionFd), buf, length, 0);
                if (writeLength != length) {
                    throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
                }
            } else {
                *_phase = FINISH;
            }
        }
    } 
    #endif

    #if 1 // CGI Part
    if (*_phase == CGI_RUN) {
        // FIXME[yekim]: _cgi를 처음에 절대 경로와 함께 생성하는데, 이를 그대로 사용하면 cgi가 동작하지 않습니다 ㅠㅠ
        delete _cgi;
        if (data._URIExtension == std::string("py")) {
            data._CGIBinary = std::string("/usr/bin/python3");
        } else if (data._URIExtension == std::string("php")) {
            data._CGIBinary = std::string("/Users/joohongpark/Desktop/webserv_workspace/webserv/main/php-bin/php-cgi");
        }
        (*_cgi) = new CGISession(data);
        (*_cgi)->makeCGIProcess();
        fcntl((*_cgi)->getOutputStream(), F_SETFL, O_NONBLOCK);
        convertHeaderMapToString(true);
        send((*_connectionFd), (*_headerString).data(), (*_headerString).length(), 0);
        *_phase = CGI_REQ;
    }
    
    if (*_phase == CGI_REQ) {
        if (data._postFilePath.empty()) {
            *_phase = CGI_RECV_LOOP;
        } else {
            (*_file) = new FileController(data._postFilePath, FileController::READ);
            *_phase = CGI_SEND_LOOP;
        }
    }
    
    if (*_phase == CGI_SEND_LOOP) {
        char buf[RESPONSE_BUFFER_SIZE];

        int length = read((*_file)->getFd(), buf, RESPONSE_BUFFER_SIZE);
        if (length != 0) {
            int writeLength = write((*_cgi)->getInputStream(), buf, length);
            if (writeLength != length) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            }
        } else {
            (*_file)->del();
            *_phase = CGI_RECV_LOOP;
        }
    }
    
    if (*_phase == CGI_RECV_LOOP) {
        char buf[RESPONSE_BUFFER_SIZE];

        int length = read((*_cgi)->getOutputStream(), buf, RESPONSE_BUFFER_SIZE);
        if (length != 0) {
            int writeLength = send(*_connectionFd, buf, length, 0);
            if (writeLength != length) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            }
        } else {
            *_phase = FINISH;
        }
    }
    #endif
    return (*_phase);
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
    return ((*_cgi)->getOutputStream());
}

std::string HTTPResponseHandler::getMIME(const std::string& extension) const {
    // FIXME : yekim : types 블록에 대해 map 타입으로 파싱해야 합니다.

    std::map<std::string, std::string> mime = (*_nginxConfig)._http.types.typeMap;
    if(extension == std::string("") || mime.find(extension) == mime.end()) {
        return (std::string("application/octet-stream"));
    } else {
        return (mime[extension]);
    }
}

#if 0
    mine[std::string("html")] = std::string("text/html");
    mine[std::string("htm")] = std::string("text/html");
    mine[std::string("shtml")] = std::string("text/html");
    mine[std::string("css")] = std::string("text/css");
    mine[std::string("xml")] = std::string("text/xml");
    mine[std::string("gif")] = std::string("image/gif");
    mine[std::string("jpeg")] = std::string("image/jpeg");
    mine[std::string("jpg")] = std::string("image/jpeg");
    mine[std::string("js")] = std::string("application/javascript");
    mine[std::string("atom")] = std::string("application/atom+xml");
    mine[std::string("rss")] = std::string("application/rss+xml");
    mine[std::string("mml")] = std::string("text/mathml");
    mine[std::string("txt")] = std::string("text/plain");
    mine[std::string("jad")] = std::string("text/vnd.sun.j2me.app-descriptor");
    mine[std::string("wml")] = std::string("text/vnd.wap.wml");
    mine[std::string("htc")] = std::string("text/x-component");
    mine[std::string("png")] = std::string("image/png");
    mine[std::string("svg")] = std::string("image/svg+xml");
    mine[std::string("svgz")] = std::string("image/svg+xml");
    mine[std::string("tif")] = std::string("image/tiff");
    mine[std::string("tiff")] = std::string("image/tiff");
    mine[std::string("wbmp")] = std::string("image/vnd.wap.wbmp");
    mine[std::string("webp")] = std::string("image/webp");
    mine[std::string("ico")] = std::string("image/x-icon");
    mine[std::string("jng")] = std::string("image/x-jng");
    mine[std::string("bmp")] = std::string("image/x-ms-bmp");
    mine[std::string("woff")] = std::string("font/woff");
    mine[std::string("woff2")] = std::string("font/woff2");
    mine[std::string("jar")] = std::string("application/java-archive");
    mine[std::string("war")] = std::string("application/java-archive");
    mine[std::string("ear")] = std::string("application/java-archive");
    mine[std::string("json")] = std::string("application/json");
    mine[std::string("hqx")] = std::string("application/mac-binhex40");
    mine[std::string("doc")] = std::string("application/msword");
    mine[std::string("pdf")] = std::string("application/pdf");
    mine[std::string("ps")] = std::string("application/postscript");
    mine[std::string("eps")] = std::string("application/postscript");
    mine[std::string("ai")] = std::string("application/postscript");
    mine[std::string("rtf")] = std::string("application/rtf");
    mine[std::string("m3u8")] = std::string("application/vnd.apple.mpegurl");
    mine[std::string("kml")] = std::string("application/vnd.google-earth.kml+xml");
    mine[std::string("kmz")] = std::string("application/vnd.google-earth.kmz");
    mine[std::string("xls")] = std::string("application/vnd.ms-excel");
    mine[std::string("eot")] = std::string("application/vnd.ms-fontobject");
    mine[std::string("ppt")] = std::string("application/vnd.ms-powerpoint");
    mine[std::string("odg")] = std::string("application/vnd.oasis.opendocument.graphics");
    mine[std::string("odp")] = std::string("application/vnd.oasis.opendocument.presentation");
    mine[std::string("ods")] = std::string("application/vnd.oasis.opendocument.spreadsheet");
    mine[std::string("odt")] = std::string("application/vnd.oasis.opendocument.text");
    mine[std::string("pptx")] = std::string("application/vnd.openxmlformats-officedocument.presentationml.presentation");
    mine[std::string("xlsx")] = std::string("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    mine[std::string("docx")] = std::string("application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    mine[std::string("wmlc")] = std::string("application/vnd.wap.wmlc");
    mine[std::string("7z")] = std::string("application/x-7z-compressed");
    mine[std::string("cco")] = std::string("application/x-cocoa");
    mine[std::string("jardiff")] = std::string("application/x-java-archive-diff");
    mine[std::string("jnlp")] = std::string("application/x-java-jnlp-file");
    mine[std::string("run")] = std::string("application/x-makeself");
    mine[std::string("pl")] = std::string("application/x-perl");
    mine[std::string("pm")] = std::string("application/x-perl");
    mine[std::string("prc")] = std::string("application/x-pilot");
    mine[std::string("pdb")] = std::string("application/x-pilot");
    mine[std::string("rar")] = std::string("application/x-rar-compressed");
    mine[std::string("rpm")] = std::string("application/x-redhat-package-manager");
    mine[std::string("sea")] = std::string("application/x-sea");
    mine[std::string("swf")] = std::string("application/x-shockwave-flash");
    mine[std::string("sit")] = std::string("application/x-stuffit");
    mine[std::string("tcl")] = std::string("application/x-tcl");
    mine[std::string("tk")] = std::string("application/x-tcl");
    mine[std::string("der")] = std::string("application/x-x509-ca-cert");
    mine[std::string("pem")] = std::string("application/x-x509-ca-cert");
    mine[std::string("crt")] = std::string("application/x-x509-ca-cert");
    mine[std::string("xpi")] = std::string("application/x-xpinstall");
    mine[std::string("xhtml")] = std::string("application/xhtml+xml");
    mine[std::string("xspf")] = std::string("application/xspf+xml");
    mine[std::string("zip")] = std::string("application/zip");
    mine[std::string("bin")] = std::string("application/octet-stream");
    mine[std::string("exe")] = std::string("application/octet-stream");
    mine[std::string("dll")] = std::string("application/octet-stream");
    mine[std::string("deb")] = std::string("application/octet-stream");
    mine[std::string("dmg")] = std::string("application/octet-stream");
    mine[std::string("iso")] = std::string("application/octet-stream");
    mine[std::string("img")] = std::string("application/octet-stream");
    mine[std::string("msi")] = std::string("application/octet-stream");
    mine[std::string("msp")] = std::string("application/octet-stream");
    mine[std::string("msm")] = std::string("application/octet-stream");
    mine[std::string("mid")] = std::string("audio/midi");
    mine[std::string("midi")] = std::string("audio/midi");
    mine[std::string("kar")] = std::string("audio/midi");
    mine[std::string("mp3")] = std::string("audio/mpeg");
    mine[std::string("ogg")] = std::string("audio/ogg");
    mine[std::string("m4a")] = std::string("audio/x-m4a");
    mine[std::string("ra")] = std::string("audio/x-realaudio");
    mine[std::string("3gpp")] = std::string("video/3gpp");
    mine[std::string("3gp")] = std::string("video/3gpp");
    mine[std::string("ts")] = std::string("video/mp2t");
    mine[std::string("mp4")] = std::string("video/mp4");
    mine[std::string("mpeg")] = std::string("video/mpeg");
    mine[std::string("mpg")] = std::string("video/mpeg");
    mine[std::string("mov")] = std::string("video/quicktime");
    mine[std::string("webm")] = std::string("video/webm");
    mine[std::string("flv")] = std::string("video/x-flv");
    mine[std::string("m4v")] = std::string("video/x-m4v");
    mine[std::string("mng")] = std::string("video/x-mng");
    mine[std::string("asx")] = std::string("video/x-ms-asf");
    mine[std::string("asf")] = std::string("video/x-ms-asf");
    mine[std::string("wmv")] = std::string("video/x-ms-wmv");
    mine[std::string("avi")] = std::string("video/x-msvideo");
#endif
