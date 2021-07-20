#ifndef NGINXCONFIG_HPP
#define NGINXCONFIG_HPP

#include "NginxParser.hpp"
namespace NginxConfig {
class NginxBlock {
    public:
        std::string rawData;
        std::map<std::string, std::string> dirMap;

        NginxBlock() {};
        NginxBlock(std::string rawData) : rawData(rawData) {};

        void checkValidNumberValue(NginxBlock& block, std::string directive);
        void checkValidErrorPage(const std::vector<std::string>& errorPage);
        void checkAutoindexValue(NginxBlock& block);
};

class NoneBlock : public NginxBlock {
    public:
        std::vector<std::string> dirCase;
        std::string user;
        std::string worker_processes;
};

class TypesBlock : public NginxBlock {
    public:
        std::map<std::string, std::string> typeMap;
        TypesBlock() {}
        TypesBlock(std::string rawData) : NginxBlock(rawData) {
            setTypesBlock();
        }
        void setTypeMap(std::map<std::string, std::string>& typeMap, std::string& type, std::string& value);        
        void setTypesBlock();
};

typedef struct s_InheritData {
    std::string root;
    std::string autoindex;
    std::string clientMaxBodySize;
    std::vector<std::string> error_page;
    std::vector<std::string> index;
} InheritData;

class LocationBlock : public NginxBlock {
    public:
        std::vector<std::string> dirCase;
        std::string _locationPath;
        std::vector<std::string> _return;
        std::vector<std::string> index;
        std::vector<std::string> error_page;
        std::vector<std::string> allowed_method;
        std::vector<std::string> inner_proxy;
        InheritData _inheritData;

        LocationBlock() {}
        LocationBlock(std::string rawData, std::string locationPath, InheritData inheritData);
        void setDirectiveTypes();
        void checkLocationBlock();
        void inheritDirectives();
        void setBlock();
};

class ServerBlock : public NginxBlock{
    public:
        std::vector<std::string> dirCase;
        std::vector<std::string> index;
        std::vector<std::string> error_page;
        std::vector<class LocationBlock> location;

        ServerBlock() {}
        ServerBlock(std::string rawData);
        void setDirectiveTypes();        
        InheritData getInheritData();
        void setBlock();
        void checkServerBlock();

};

class HttpBlock : public NginxBlock{
    public:
        std::vector<std::string> dirCase;
        std::vector<class ServerBlock> server;
        class TypesBlock types;
    
        HttpBlock() {}
        HttpBlock(std::string rawData);
        void setDirectiveTypes();
        void setBlock();
        void checkHttpBlock();
};

class GlobalConfig : public NginxParser {
    public:
    public:
        class NoneBlock _none;
        class HttpBlock _http;

        GlobalConfig(const std::string& fileName);

};
}
#endif