#include "Parser.hpp"

Parser::Parser(const std::string& fileName) : _fileName(fileName) {
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

Parser::~Parser(){
}

void Parser::showRawData() const {
    std::cout << this->_rawData << std::endl;
}

std::string Parser::getRawData() const {
    return (this->_rawData);
}

std::string Parser::parseOneLine(std::size_t &pos) {
    std::size_t posNewLine = this->_rawData.find("\n", pos);
    if (posNewLine == std::string::npos)
        return "";
    return (this->_rawData.substr(pos, posNewLine - pos));
}

std::string Parser::parseComment(std::size_t& pos) {
    std::size_t posNewLine = this->_rawData.find("\n", pos);
    std::size_t posHash = this->_rawData.find("#", pos);
    if (posNewLine == std::string::npos)
        return "";
    if (posHash < posNewLine) {
        std::size_t commentLen = posNewLine - posHash;
        pos = posNewLine + 1;
        return (this->_rawData.substr(posHash, commentLen));
    }
    return (this->_rawData);
}

std::string Parser::parseStringHeadByDelimiter(std::size_t& pos, const std::string& needle) {
    std::size_t found = this->_rawData.find(needle, pos);
    if (found == std::string::npos)
        return "";
    std::string strHead = this->_rawData.substr(pos, found - pos);
    pos = found + needle.size();
    return strHead;
}

std::string Parser::parseStringHeadByDelimiter(const std::string& str, std::size_t& pos, const std::string& needle) {
    std::size_t found = str.find(needle, pos);
    if (found == std::string::npos)
        return "";
    std::string strHead = str.substr(pos, found - pos);
    pos = found + needle.size();
    return strHead;
}

std::string Parser::leftSpaceTrim(std::string s, const std::string& drop) {
    return s.erase(0,s.find_first_not_of(drop));
}

std::string Parser::rightSpaceTrim(std::string s, const std::string& drop) {
    return s.erase(s.find_last_not_of(drop)+1);
}