#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>

class Parser {
protected:
    Parser();
    std::string _rawData;
    std::string _fileName;
    std::map<std::string, void *> _configMap; 

public:
    Parser(const std::string& fileName);
    virtual ~Parser();

    std::string getRawData() const;
    std::string parseComment(std::size_t& pos);
    std::string parseOneLine(std::size_t& pos);
    std::string parseStringHeadByDelimiter(std::size_t& pos, const std::string &needle);
    std::string parseStringHeadByDelimiter(const std::string& buf, std::size_t& pos, const std::string &needle);
    void showRawData() const;
    virtual void showConfigurationMap() = 0;

    std::string leftSpaceTrim(std::string s, const std::string& drop = " ");
    std::string rightSpaceTrim(std::string s, const std::string& drop = " ");
};

#endif