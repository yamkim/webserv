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
        Parser(const std::string& fileName) : _fileName(fileName) {
            _rawData = "";
            std::ifstream readFile;

            readFile.open(this->_fileName);
            if (!readFile.is_open()){
                throw std::string("Error: Configuration open error.");
            }
            while (!readFile.eof()) {
                std::string tmp;
                getline(readFile, tmp);
                this->_rawData += tmp;
                this->_rawData += "\n";
            }
            readFile.close();

            if (!isValidBlockSet(this->_rawData)) {
                throw std::string("Error: bracket pair is not matched.");
            }
        }
        virtual ~Parser(){};

        const std::string& getRawData() const{
            return _rawData;
        }

        bool isValidBlockSet (const std::string& buf) {
            std::size_t pos = 0;

            int leftBracketNum = 0;
            int rightBracketNum = 0;

            while (buf[pos]) {
                if (buf[pos] == '{') {
                    leftBracketNum++;
                } else if (buf[pos] == '}') {
                    rightBracketNum++;
                }
                ++pos;
            }
            return (leftBracketNum == rightBracketNum);
        }

        std::string leftSpaceTrim(std::string s) {
            const std::string drop = " ";
            return s.erase(0, s.find_first_not_of(drop));
        }

        std::string rightSpaceTrim(std::string s) {
            const std::string drop = " ";
            return s.erase(s.find_last_not_of(drop)+1);
        }

        static std::string sideSpaceTrim(std::string s) {
            const std::string drop = " ";
            std::string ret = s.erase(s.find_last_not_of(drop)+1);
            ret = ret.erase(0, ret.find_first_not_of(drop));
            return ret;
        }

        static bool isCharInString(const std::string& str, const char c) {
            std::size_t itr = 0;
            while (str[itr]) {
                if (str[itr] == c)
                    return true;
                ++itr;
            }
            return false;
        } 

        // 매개변수 delimiter의 요소라면, 이를 기준으로 split 후에 첫 단어만 가지고 옵니다.
        // [          listen     5000;]이고, delimit이 " "라면, listen만 가지고 옵니다.
        // 찾으려는 delimiter가 없는 경우, error를 반환합니다.
        // ex) text/css    css => ';'이 없는 경우로, str
        // 문자열에 delimiter가 있어야하는 경우 checker를 true로 하여, 해당하지 않는 경우 거릅니다.
        static std::string	getIdentifier(const std::string str, std::size_t& endPos, std::string delimiter, bool checker)
        {
            size_t wordSize = 0;

            if (checker && str.find(delimiter) == std::string::npos) {
                std::cout << "[DEBUG] String: " << str << std::endl;
                throw std::string("Error: There is no delimiter[" + delimiter + "]. Parser::getIdentifier");
                // throw ErrorHandler(std::string("Error: There is no delimiter["+ delimiter + "].").c_str(), ErrorHandler::CRITICAL, "Parser::getIdentifier");
            }

            while ((str[endPos] != '\0') && isCharInString(delimiter, str[endPos])) {
                ++endPos;
            }
            size_t begPos = endPos;
            while ((str[endPos] != '\0') && !isCharInString(delimiter, str[endPos])) {
                ++wordSize;
                ++endPos;
            }

            return (str.substr(begPos, wordSize));
        }

        static std::vector<std::string> getSplitBySpace(std::string str) {
            std::vector<std::string> ret;

            std::size_t pos = 0;
            // std::cout << "[DEBUG] tmpLine with space: " << str << std::endl; 
            while (pos < str.size()) {
                // std::string tmp = getIdentifier(str, pos, " \r\n");
                std::string tmp = getIdentifier(str, pos, " ", false);
                if (tmp.empty()) {
                    break ;
                }
                ret.push_back(tmp);
            }
            return ret; 
        }

        static bool isNumber(const std::string& str) {
            int pos = 0;
            while (str[pos]) {
                if (!(str[pos] >= '0' && str[pos] <= '9')) {
                    return false;
                }
                ++pos;
            }
            return true;
        }
};
#endif