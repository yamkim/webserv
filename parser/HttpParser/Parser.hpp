#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <map>

class Parser {
private:
    std::string _rawData;
    std::string _fileName;
    std::map<std::string, std::string> _configMap; 

public:
    Parser(const std::string fileName);

    void setConfigurationMap();

    std::string getStringHeadByDelimiter(const std::string &buf, std::size_t &pos, const std::string &needle);
    void showRawData() const;
    void showConfigurationMap();
};

#endif