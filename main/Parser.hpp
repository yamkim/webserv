#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <map>
#include <vector>
#include <iostream>

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
            if (!readFile.is_open())
                throw std::string("Error: File open error.");
            while (!readFile.eof()) {
                std::string tmp;
                getline(readFile, tmp);
                this->_rawData += tmp;
                this->_rawData += "\n";
            }
            readFile.close();

            if (!isValidBlockSet(this->_rawData)) {
                throw std::string("Error: bracket pair is not valid.");
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

        std::string sideSpaceTrim(std::string s) {
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
        static std::string	getIdentifier(const std::string str, std::size_t& endPos, std::string delimiter)
        {
            size_t wordSize = 0;

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
            while (str[pos]) {
                std::string tmp = getIdentifier(str, pos, " \r\n");
                if (tmp.empty()) {
                    break ;
                }
                std::cout << "tmp: " << tmp << std::endl;
                ret.push_back(tmp);
            }
            return ret; 
        }
};
#endif