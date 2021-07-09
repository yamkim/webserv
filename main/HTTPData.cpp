#include <HTTPData.hpp>

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
    std::size_t foundQuestion = _reqURI.rfind("?");
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
