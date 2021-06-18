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

Parser::~Parser(){
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