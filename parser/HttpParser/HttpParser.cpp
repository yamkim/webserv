#include "HttpParser.hpp"

HttpParser::HttpParser(std::string fileName) : Parser(fileName) {
}

HttpParser::~HttpParser(){
}

void HttpParser::setConfigurationMap() {
    std::size_t pos = 0;

    std::string method = getStringHeadByDelimiter(this->_rawData, pos, "\n");
    this->_configMap["First-Line"] = method;

    std::string key, value;
    while ((key = getStringHeadByDelimiter(this->_rawData, pos, ": ")) != "") {
        value = getStringHeadByDelimiter(this->_rawData, pos, "\n");
        this->_configMap[key] = value;
    }
}

void HttpParser::showConfigurationMap() {
    std::map<std::string, std::string>::iterator iter;
    for (iter = this->_configMap.begin(); iter != this->_configMap.end(); ++iter) {
        std::cout << "[" << iter->first << "]: " << iter->second << std::endl;
    }
}
