#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include "ErrorHandler.hpp"

class Parser {
    protected:
        Parser();
        std::string _rawData;
        std::string _fileName;

    public:
        Parser(const std::string& fileName);
        ~Parser();
        const std::string& getRawData() const;
        bool isValidBlockSet (const std::string& buf);
        std::string leftSpaceTrim(std::string s);
        std::string rightSpaceTrim(std::string s);
        static std::string sideSpaceTrim(std::string s);
        static bool isCharInString(const std::string& str, const char c);
        static std::string	getIdentifier(const std::string str, std::size_t& endPos, std::string delimiter, bool checker);
        static std::vector<std::string> getSplitBySpace(std::string str);
        static bool isNumber(const std::string& str);
};
#endif