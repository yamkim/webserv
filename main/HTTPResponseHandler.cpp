#include "HTTPResponseHandler.hpp"

// FIXME : 리스폰스에 URI 외에 더 다양한 아규먼트를 집어넣어야 하는데 어떤 형식으로 집어넣을지 고민 중입니다. 추후에 수정하겠습니다.
HTTPResponseHandler::HTTPResponseHandler(int connectionFd, std::string arg) : HTTPHandler(connectionFd) {
    _phase = RESOURCE_OPEN;
    _URI = arg;
    // FIXME : root 경로와 같은 정보는 .conf 파일에서 받아와야 합니다.
    _root = std::string("./html");
}

HTTPResponseHandler::~HTTPResponseHandler() {}

HTTPResponseHandler::Phase HTTPResponseHandler::process(void) {
    if (_phase == RESOURCE_OPEN) {
        // TODO: joopark - CGI 처리 로직 추가해보기 (리팩토링은 추후에)
        // NOTE : 서버 자원에 대한 요청이 유효한지를 검사하는 로직이고 자원(파일)이 존재하면 파일을 엽니다. 추후 cgi 관련 핸들링도 추가 예정입니다.
        std::string absolutePath = _root;
        absolutePath += _URI;
        if (_URI.at(_URI.length() - 1) == '/') {
            // NOTE : 추후 .conf 파일에 기재된 autoindex 여부와 index 파일 처리 여부에 따라 달라집니다.
            absolutePath += std::string("index.html");
        }
        file.open(absolutePath, std::ios_base::in);
        std::cout << "[DEBUG] Request URI : " << absolutePath << std::endl;
        _phase = ASSEMBLE_HEADER;
    } else if (_phase == ASSEMBLE_HEADER) {
        if (file.isOpen()) {
            // NOTE : 이 부분도 서버의 상태나 request 된 내용 등을 참조해서 다양하게 처리해야 합니다.
            buildGeneralHeader("HTTP/1.1 200 OK");
            std::string extension = getExtenstion(_URI);
            if (extension.empty()) {
                buildOKHeader("text/html", file.length());
            } else {
                buildOKHeader(getMIME(extension), file.length());
            }
            _phase = OK;
        } else {
            buildGeneralHeader("HTTP/1.1 404 Not Found");
            _internalHTMLString = get404Body();
            buildOKHeader("text/html", _internalHTMLString.length());
            _phase = NOT_FOUND;
        }
        convertHeaderMapToString();
        size_t writeLength = send(_connectionFd, _headerString.data(), _headerString.length(), 0);
        if (writeLength != _headerString.length()) {
            // FIXME : 서버가 한번에 버퍼 사이즈만큼 다 못보낼수도 있음. 수정 필요
            throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
        }
    } else if (_phase == OK) {
        char buf[RESPONSE_BUFFER_SIZE];
        int length = file.read(buf, RESPONSE_BUFFER_SIZE);
        if (length != 0) {
            int writeLength = send(_connectionFd, buf, length, 0);
            if (writeLength != length) {
                throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
            }
        } else {
            _phase = FINISH;
        }
    } else if (_phase == NOT_FOUND) {
        size_t writeLength = send(_connectionFd, _internalHTMLString.c_str(), _internalHTMLString.length(), 0);
        if (writeLength != _internalHTMLString.length()) {
            throw ErrorHandler("Error: send error.", ErrorHandler::ALERT, "HTTPResponseHandler::process");
        } else {
            _phase = FINISH;
        }
    }
    return _phase;
}

void HTTPResponseHandler::buildGeneralHeader(std::string status) {
    static char timeBuffer[48];
    time_t rawtime;

    std::time(&rawtime);
    struct tm* timeinfo = std::gmtime(&rawtime);
    std::strftime(timeBuffer, 48, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);

    _headerString = status;
    _headerString += "\r\n";

    // NOTE https://developer.mozilla.org/ko/docs/Web/HTTP/Headers
    // 서버의 소프트웨어 정보
    _headers["Server"] = std::string("webserv/0.1");
    // HTTP 메시지가 만들어진 날짜와 시간
    _headers["Date"] = std::string(timeBuffer);
    // 전송이 완료된 후 네트워크 접속을 유지할지 말지
    // REVIEW 상황따라 keep-alive / close가 갈리긴 하지만 현재 구현상으로는 메시지 보내고 끊김. keep-alive 형태로 현재 연결된 fd를 유지시켜야 하나?
    _headers["Connection"] = std::string("close");
}

void HTTPResponseHandler::buildOKHeader(std::string type, long contentLength) {
    std::stringstream ssLength;

    ssLength << contentLength;
    _headers["Content-Type"] = type;
    _headers["Content-Length"] = ssLength.str();
}

void HTTPResponseHandler::convertHeaderMapToString(void) {
    std::map<std::string, std::string>::iterator iter;
    for (iter = _headers.begin(); iter != _headers.end(); ++iter) {
        _headerString += iter->first;
        _headerString += ": ";
        _headerString += iter->second;
        _headerString += "\r\n";
    }
    _headerString += "\r\n";
}

std::string& HTTPResponseHandler::get404Body(void) {
    // FIXME : += 연산자 쓰면 자꾸 append 되는데 이거 말고 더 좋은 방법이 있을까요...? ㅠㅠ 아니면 다른곳에 두는것도 좋은거 같습니다.
    static std::string httpString = "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr><center>webserv/0.0.1</center></body></html>";
    return (httpString);
}

std::string HTTPResponseHandler::getMIME(std::string extension) {
    // FIXME : 추후에 nginx.conf 파일에서 파싱해 와야 합니다. map 형식의 키-벨류 형식을 그대로 유지해야 할 듯 합니다.
    std::map<std::string, std::string> mine;

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

    if(mine.find(extension) == mine.end()) {
        return (std::string("application/octet-stream"));
    } else {
        return (mine[extension]);
    }
}

std::string HTTPResponseHandler::getExtenstion(std::string& URI) {
    // REVIEW : 이 기능을 request핸들러로 옮기는 것 검토
    std::string strHead;
    
    std::size_t foundDot = URI.rfind(".");
    std::size_t foundSlash = URI.rfind("/");
    if (foundDot == std::string::npos || foundSlash > foundDot) {
        return (std::string(""));
    } else {
        return (URI.substr(foundDot + 1));
    }
}
