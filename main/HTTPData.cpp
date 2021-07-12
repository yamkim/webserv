#include <HTTPData.hpp>

HTTPData::HTTPData() {
    setResStartLineMap();
}

std::string HTTPData::getMethod(void) const {
    return (_reqMethod);
}

std::string HTTPData::getURI(void) const {
    return (_reqURI);
}

std::string HTTPData::getExtension(std::string URI) {
    std::string ret;
    std::size_t foundDot = URI.rfind(".");
    std::size_t foundSlash = URI.rfind("/");
    if (foundDot == std::string::npos 
        || (foundSlash != std::string::npos && foundSlash > foundDot)) {
        return (std::string(""));
    }
    ret = URI.substr(foundDot + 1);
    if (ret[ret.size() - 1] == '$')
        ret = ret.substr(0, ret.size() - 1);
    return ret;
}

void HTTPData::setURIelements(void) {
    std::size_t foundQuestion = _reqURI.find("?");
    if (foundQuestion == std::string::npos) {
        _URIFilePath = _reqURI;
    } else {
        _URIQueryString = _reqURI.substr(foundQuestion + 1);
        _URIFilePath = _reqURI.substr(0, foundQuestion);
    }
    std::size_t foundDot = _URIFilePath.rfind(".");
    std::size_t foundSlash = _URIFilePath.rfind("/");
    _URIFileName = _URIFilePath.substr(foundSlash + 1);
    if (foundDot != std::string::npos && foundSlash <= foundDot) {
        _URIExtension = _URIFilePath.substr(foundDot + 1);
    }
}

void HTTPData::setResStartLineMap(void) {
	_resStartLineMap[101] = "Switching Protocols";
	_resStartLineMap[102] = "Processing";
	_resStartLineMap[200] = "OK";
	_resStartLineMap[201] = "Created";
	_resStartLineMap[202] = "Accepted";
	_resStartLineMap[203] = "Non-authoritative Information";
	_resStartLineMap[204] = "No Content";
	_resStartLineMap[205] = "Reset Content";
	_resStartLineMap[206] = "Partial Content";
	_resStartLineMap[207] = "Multi-Status";
	_resStartLineMap[208] = "Already Reported";
	_resStartLineMap[226] = "IM Used";
	_resStartLineMap[300] = "Multiple Choices";
	_resStartLineMap[301] = "Moved Permanently";
	_resStartLineMap[302] = "Found";
	_resStartLineMap[303] = "See Other";
	_resStartLineMap[304] = "Not Modified";
	_resStartLineMap[305] = "Use Proxy";
	_resStartLineMap[307] = "Temporary Redirect";
	_resStartLineMap[308] = "Permanent Redirect";
	_resStartLineMap[400] = "Bad Request";
	_resStartLineMap[401] = "Unauthorized";
	_resStartLineMap[402] = "Payment Required";
	_resStartLineMap[403] = "Forbidden";
	_resStartLineMap[404] = "Not found";
	_resStartLineMap[405] = "Method Not Allowed";
	_resStartLineMap[406] = "Not Acceptable";
	_resStartLineMap[407] = "Proxy Authentication Required";
	_resStartLineMap[408] = "Required Timeout";
	_resStartLineMap[409] = "Conflict";
	_resStartLineMap[410] = "Gone";
	_resStartLineMap[411] = "Length Required";
	_resStartLineMap[412] = "Precondition Failed";
	_resStartLineMap[413] = "Request Entity Too Large";
	_resStartLineMap[414] = "Request URI Too Long";
	_resStartLineMap[415] = "Unsupported Media Type";
	_resStartLineMap[416] = "Requested Range Not Satisfiable";
	_resStartLineMap[417] = "Expectation Failed";
	_resStartLineMap[418] = "IM_A_TEAPOT";
	_resStartLineMap[500] = "Internal Server Error";
	_resStartLineMap[501] = "Not Implemented";
	_resStartLineMap[502] = "Bad Gateway";
	_resStartLineMap[503] = "Service Unavailable";
	_resStartLineMap[504] = "Gateway Timeout";
	_resStartLineMap[505] = "HTTP Version Not Supported";
	_resStartLineMap[506] = "Variant Also Negotiates";
	_resStartLineMap[507] = "Insufficient Storage";
	_resStartLineMap[508] = "Loop Detected";
	_resStartLineMap[510] = "Not Extened";
	_resStartLineMap[511] = "Network Authentication Required";
	_resStartLineMap[599] = "Network Connect Timeout Error";
}
