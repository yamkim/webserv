#include <iostream>
#include "Parser.hpp"

#include "NginxConfig.hpp"
class ConfigParser : public Parser{
    private:
        ConfigParser();
        NginxConfig _nginxConfig;

    public:
        ConfigParser(const std::string& fileName);
        virtual ~ConfigParser();

        NginxConfig getNginxConfig() const;

        void parseDirective(std::size_t &pos);
        void setConfigurationMap();
        void showConfigurationMap();
        std::pair<std::string, std::string> parseNoneBlockDirective(const std::string& str);
        void parseBlockDirective(const std::string& blockName, const std::string& blockContent);
};

ConfigParser::ConfigParser(const std::string& fileName) : Parser(fileName) {
}

ConfigParser::~ConfigParser(){
}

NginxConfig ConfigParser::getNginxConfig() const {
    return _nginxConfig;
}

std::pair<std::string, std::string> ConfigParser::parseNoneBlockDirective(const std::string& str) {
    std::size_t pos = 0;
    std::string directive = this->parseStringHeadByDelimiter(str, pos, " ");
    std::string content;
    if (directive.empty()) {    // 요소가 하나인 경우..
        throw std::string("Wrong format. Only Directive");
    } else {                    // 그냥 directive인 경우
        content = this->parseStringHeadByDelimiter(str, pos, ";");
    }
    return make_pair(directive, content);
}

void ConfigParser::parseBlockDirective(const std::string& blockName, const std::string& blockContent) {
    std::size_t tmpPos = 0;
    std::string tmpLine = this->parseStringHeadByDelimiter(blockContent, tmpPos, "\n");
    while (!tmpLine.empty()) { // block을 모두 탐색한 경우
        std::cout << "Parse line: " << tmpLine << std::endl;
        tmpLine = this->leftSpaceTrim(tmpLine, " ");
        std::size_t tmp = 0;
        std::string tmpDir = this->parseStringHeadByDelimiter(tmpLine, tmp, " ");

        std::pair<std::string, std::string> tmpDirPair;
        if (this->_nginxConfig.isBlockDirective(blockName, tmpDir)) {
            tmpDirPair = this->parseNoneBlockDirective(tmpLine);
            // std::cout << "$ tmpDirPair.first: " << tmpDirPair.first << ", tmpDirPair.second: " << tmpDirPair.second << std::endl;
            this->_nginxConfig.setDirectiveValue(blockName, tmpDirPair.first, tmpDirPair.second);
        } else {
            throw std::string("Invalid Directive in " + blockName + " block: " + tmpDirPair.first);
        }
        tmpLine = this->parseStringHeadByDelimiter(blockContent, tmpPos, "\n");
    }
}

void ConfigParser::parseDirective(std::size_t &pos) {
    std::string tmpLine = this->parseStringHeadByDelimiter(pos, "\n");
    if (tmpLine.empty()) {
        std::cout << "empty line" << std::endl;
        return ;
    }

    std::pair<std::string, std::string> tmpDirPair;
    std::size_t tmpPos = 0;
    std::string tmpDir = this->parseStringHeadByDelimiter(tmpLine, tmpPos, "{");
    if (!tmpDir.empty()) {
        std::string blockName = tmpDir;
        blockName = this->leftSpaceTrim(blockName, " ");
        blockName = this->rightSpaceTrim(blockName, " ");
        std::string blockContent = this->parseStringHeadByDelimiter(pos, "}");
        // std::cout << "$ blockName: " << blockName << ", blockContent: " << blockContent << std::endl;
        this->parseBlockDirective(blockName, blockContent);
        return ;
    } else {
        tmpPos = 0;
        tmpDir = this->parseStringHeadByDelimiter(tmpLine, tmpPos, " ");
        tmpDirPair = this->parseNoneBlockDirective(tmpLine);
        if (this->_nginxConfig.isBlockDirective("none", tmpDir)) {
            tmpDirPair = this->parseNoneBlockDirective(tmpLine);
            this->_nginxConfig.setDirectiveValue("none", tmpDirPair.first, tmpDirPair.second);
        } else {
            throw std::string("Invalid Directive in none block: " + tmpDirPair.first);
        }
    }
}

void ConfigParser::showConfigurationMap() {
    std::cout << "show configuration map" << std::endl;
}

int main(void) {
    try {
        ConfigParser parser("nginx.conf");
        // parser.showRawData();
        std::size_t pos = 0;
//        std::cout << parser.parseComment(pos) << std::endl;
        parser.parseDirective(pos);
        std::cout << parser.getNginxConfig().getDirectiveValue("http", "charset") << std::endl;
        std::cout << parser.getNginxConfig().getDirectiveValue("http", "include") << std::endl;
        std::cout << parser.getNginxConfig().getDirectiveValue("http", "default_type") << std::endl;
        #if 0
        std::cout << parser.getNginxConfig().getDirectiveValue("none", "user") << std::endl;
        std::cout << parser.getNginxConfig().getDirectiveValue("none", "worker_processes") << std::endl;
        std::cout << parser.getNginxConfig().getDirectiveValue("events", "worker_connections") << std::endl;
        std::cout << parser.getNginxConfig().getDirectiveValue("http", "charset") << std::endl;
        std::cout << parser.getNginxConfig().getDirectiveValue("http", "include") << std::endl;
        std::cout << parser.getNginxConfig().getDirectiveValue("http", "default_type") << std::endl;
        #endif

    } catch (const std::string& error) {
        std::cout << error << std::endl;
    }

    return (0);
}
