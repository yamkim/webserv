#include <iostream>
#include "Parser.hpp"

class NginxConfig {
    std::map<std::string, std::string> noneBlock;
    std::map<std::string, std::string> events;
    std::map<std::string, std::string> events;

};

class ConfigParser : public Parser{
    private:
        ConfigParser();

    public:
        ConfigParser(const std::string& fileName);
        virtual ~ConfigParser();

        void parseDirective(std::size_t &pos);
        void setConfigurationMap();
        void showConfigurationMap();
        std::pair<std::string, std::string> parseNoneBlockDirective(const std::string& str, std::size_t& pos);
};

ConfigParser::ConfigParser(const std::string& fileName) : Parser(fileName) {
    // None Block Directives
    std::map<std::string, std::string> noneBlock;
    _configMap["none"] = &noneBlock;
    noneBlock["user"] = "";
    noneBlock["worker_processes"] = "";

    // events block Directives
    std::map<std::string, std::string> eventsBlock;
    _configMap["events"] = &eventsBlock;
    eventsBlock["worker_connections"] = "";

    // Http block Directives
    std::map<std::string, std::string> httpBlock;
    _configMap["http"] = &httpBlock;
    httpBlock["charset"] = "";
    httpBlock["include"] = "";
    httpBlock["default_type"] = "";

    // Upstream block Directives
    std::map<std::string, std::string> upstreamBlock;
    _configMap["upstream"] = &upstreamBlock;
    upstreamBlock["server"] = "";
    upstreamBlock["keepalive"] = "";

    // server block Directives
    std::map<std::string, void *> serverBlock;
    _configMap["server"] = &serverBlock;
    serverBlock["listen"] = nullptr;
    serverBlock["server_name"] = nullptr;
    serverBlock["include"] = nullptr;
    std::map<std::string, std::string> locationBlock;
    serverBlock["location"] = &locationBlock;
    locationBlock["proxy_pass"] = "";
}

ConfigParser::~ConfigParser(){
}

std::pair<std::string, std::string> ConfigParser::parseNoneBlockDirective(const std::string& str, std::size_t& pos) {
    std::string directive = this->parseStringHeadByDelimiter(str, pos, " ");
    std::string content;
    if (directive.empty()) {   // 요소가 하나인 경우..
        throw "Wrong format. Only Directive";
    } else {                // 그냥 directive인 경우
        if (isInConfigMap(directive)) {
            content = this->parseStringHeadByDelimiter(str, pos, ";");
        } else {
            throw ("not exist directive: " + directive);
        }
    }
    return make_pair(directive, content);
}

void ConfigParser::parseDirective(std::size_t &pos) {
    std::pair<std::string, std::string> tmpDirPair;
    std::string tmpLine = this->parseStringHeadByDelimiter(pos, "\n");
    if (tmpLine.empty())
        return ;

    std::size_t tmpPos = 0;
    std::string tmpContent;
    std::string tmpDir = this->parseStringHeadByDelimiter(tmpLine, tmpPos, "{");
    if (tmpDir.empty()) {       // block directive가 아닌 경우
        tmpDirPair = this->parseNoneBlockDirective(tmpLine, tmpPos);

        _configMap[tmpDirPair.first] = reinterpret_cast(std::string)
    } else {                // block directive인 경우
        if (isInConfigMap(tmpDir)) {
            tmpDirMap = this->parseNoneBlockDirective(tmpLine, tmpPos);
        } else {
            throw ("not exist directive: " + tmpDir) ;
        }
    }

    std::map<std::string, std::string>::iterator iter;
    for (iter = tmpDirMap.begin(); iter != tmpDirMap.end(); ++iter)
        std::cout << "[" << iter->first << "]:" << iter->second << std::endl;
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
        parser.parseDirective(pos);
        parser.parseDirective(pos);

    } catch (const std::string& error) {
        std::cout << error << std::endl;
    }

    return (0);
}
