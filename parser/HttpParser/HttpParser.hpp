#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <iostream>
#include <fstream>
#include <map>
#include "Parser.hpp"

class HttpParser : public Parser {
private:
    HttpParser();
    std::map<std::string, std::string> _configMap; 

public:
    HttpParser(const std::string fileName);
    virtual ~HttpParser();

    void setConfigurationMap();
    void showConfigurationMap();
};

#endif