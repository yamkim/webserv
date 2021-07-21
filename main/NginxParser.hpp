#ifndef NGINXPARSER_HPP
#define NGINXPARSER_HPP

#include "Parser.hpp"

class NginxParser : public Parser {
    public:        
        NginxParser(const std::string& fileName);
        static void skipComment(const std::string& str, std::size_t &commentPos);
        static void findBlockSet(const std::string& buf, std::stack<int>st, std::vector<std::pair<std::string, std::size_t> >& vec, std::size_t& pos);
        static std::string getBlockContent(const std::string& buf, std::size_t& pos);
};
#endif