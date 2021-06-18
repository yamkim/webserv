#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <map>

class Parser {
protected:
    Parser();
    std::string _rawData;
    std::string _fileName;
    std::map<std::string, std::string> _configMap; 

public:
    Parser(const std::string fileName);
    virtual ~Parser();

    std::string getStringHeadByDelimiter(const std::string &buf, std::size_t &pos, const std::string &needle);
    void showRawData() const;
    virtual void showConfigurationMap() = 0;
};

#endif