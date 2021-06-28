#ifndef NGINXPARSER_HPP
#define NGINXPARSER_HPP

#include "Parser.hpp"

class NginxParser : public Parser {
    private:

    public:
        NginxParser(const std::string& fileName) : Parser(fileName) {
        }

        void skipComment (const std::string& str, std::size_t &commentPos) {
            while (str[commentPos]) {
                if (str[commentPos] != '\n') {
                    ++commentPos;
                } else {
                    return ;
                }
            }
        }

        std::string	getIdentifier(const std::string str, std::size_t& endPos, std::string delimiter)
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

        void findBlockSet(const std::string& buf, std::stack<int>st, std::vector<std::pair<std::string, std::size_t> >& vec, std::size_t& pos) {
            while (buf[pos] != '\0' && buf[pos] != '{' && buf[pos] != '}') {
                ++pos;
            }
            if (buf[pos] == '{') {
                st.push(1);
                vec.push_back(std::make_pair("{", pos));
            } else if (buf[pos] == '}') {
                if (st.top() == 1) {
                    st.pop();
                }
                vec.push_back(std::make_pair("}", pos));
            }
            if (st.empty()) {
                return ;
            }
            pos += 1;
            findBlockSet(buf, st, vec, pos);
        }

        std::string getBlockContent(const std::string& buf, std::size_t& pos) {
            std::vector<std::pair<std::string, std::size_t> > blockSet;
            std::stack<int> st;

            findBlockSet(buf, st, blockSet, pos);
            std::size_t blockBeg = blockSet.begin()->second;
            std::size_t blockEnd = (blockSet.end() - 1)->second;
            pos = blockEnd + 1;
            // std::cout << "beg: " << blockBeg << ", end: " << blockEnd << std::endl;

            return buf.substr(blockBeg + 1, blockEnd - blockBeg - 1);
        }
};
#endif