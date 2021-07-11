#ifndef NGINXBLOCK_HPP
#define NGINXBLOCK_HPP

#include "Parser.hpp"

class NginxBlock {
    public:
        std::string rawData;
        std::map<std::string, std::string> dirMap;

        NginxBlock() {};
        NginxBlock(std::string rawData) : rawData(rawData) {};

        void checkValidNumberValue(NginxBlock& block, std::string directive) {
            if (!block.dirMap[directive].empty() && !Parser::isNumber(block.dirMap[directive])) {
                throw std::string("Error: invalid value in " + directive + " directive.");
            }
        }

        void checkValidErrorPage(const std::vector<std::string>& errorPage) {
            std::vector<std::string>::const_iterator iter;
            if (!errorPage.empty()) {
                for (iter = errorPage.begin(); iter != errorPage.end() - 1; ++iter) {
                    if (!Parser::isNumber(*iter)) {
                        throw std::string("Error: invalid value " + *iter + " in error_page directive.");
                    }
                }
            }
        }

        void checkAutoindexValue(NginxBlock& block) {
            if (!block.dirMap["autoindex"].empty() && !(block.dirMap["autoindex"] == "on"
                  || this->dirMap["autoindex"] == "off")) {
                throw std::string("Error: invalid argument for autoindex directive.");
            }
        }
};
#endif