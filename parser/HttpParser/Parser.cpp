#include "Parser.hpp"

Parser::Parser(const std::string fileName) : _fileName(fileName) {
    _rawData = "";
    std::ifstream readFile;

    readFile.open(this->_fileName);
    if (!readFile.is_open())
        throw "Error: File open error.";
    while (!readFile.eof()) {
        std::string tmp;
        getline(readFile, tmp);
        this->_rawData += tmp;
        this->_rawData += "\n";
    }
    readFile.close();
}

void Parser::showRawData() const {
    std::cout << this->_rawData << std::endl;
}

std::string Parser::getStringHeadByDelimiter(const std::string &buf, std::size_t &pos, const std::string &needle) {
    std::size_t found = buf.find(needle, pos);
    if (found == std::string::npos)
        return "";
    std::string strHead = buf.substr(pos, found - pos);
    pos = found + needle.size();
    return strHead;
}

void Parser::setConfigurationMap() {
    std::size_t pos = 0;

    std::string method = getStringHeadByDelimiter(_rawData, pos, "\n");
    this->_configMap["First-Line"] = method;

    std::string key, value;
    while ((key = getStringHeadByDelimiter(_rawData, pos, ": ")) != "") {
        value = getStringHeadByDelimiter(_rawData, pos, "\n");
        this->_configMap[key] = value;
    }
}

void Parser::showConfigurationMap() {
    std::map<std::string, std::string>::iterator iter;
    for (iter = this->_configMap.begin(); iter != this->_configMap.end(); ++iter) {
        std::cout << "[" << iter->first << "]: " << iter->second << std::endl;
    }
}
