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

        void setTypeMap(std::map<std::string, std::string>& typeMap, std::string& type, std::string& value) {
            std::vector<std::string> tmpVec = Parser::getSplitBySpace(value);
            for (std::size_t i = 0; i < tmpVec.size(); ++i) {
                typeMap[tmpVec[i]] = type;
            }
        }

        void setTypesBlock() {
            std::string buf = this->rawData;
            std::size_t pos = 0;
            while (pos < buf.size()) {
                std::string tmpLine = Parser::getIdentifier(buf, pos, "\n", false);
                if (Parser::sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = Parser::getIdentifier(tmpLine, tmpPos, " ", true);
                std::string value = Parser::sideSpaceTrim(Parser::getIdentifier(tmpLine, tmpPos, ";", true));
                // std::cout << "[DEBUG] value in tmpLine: [" << value << "]" << std::endl;
                setTypeMap(this->typeMap, tmpDir, value);
            }
        }
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
        // FIXME: yekim: cgi 파라미터 관련 요소 (cgi_pass) 추가
        std::string _locationPath;
        std::vector<std::string> _return;
        std::vector<std::string> index;
        std::vector<std::string> error_page;
        std::vector<std::string> allowed_method;
        std::vector<std::string> inner_proxy;
        InheritData _inheritData;

        LocationBlock() {}
        LocationBlock(std::string rawData, std::string locationPath, InheritData inheritData) : NginxBlock(rawData), _locationPath(locationPath), _inheritData(inheritData) {
            setDirectiveTypes();
            setBlock();
            checkLocationBlock();
            inheritDirectives();
        }

        void setDirectiveTypes() {
            this->dirCase.push_back("root");
            this->dirCase.push_back("index");
            this->dirCase.push_back("autoindex");
            this->dirCase.push_back("error_page");
            this->dirCase.push_back("client_max_body_size");

            this->dirCase.push_back("return");
            this->dirCase.push_back("try_files");
            this->dirCase.push_back("deny");
            this->dirCase.push_back("error_page");
            this->dirCase.push_back("cgi_pass");
            this->dirCase.push_back("allowed_method");
            this->dirCase.push_back("inner_proxy");
        }

        void checkLocationBlock() {
            checkValidErrorPage(this->error_page);
            checkAutoindexValue(*this);
            checkValidNumberValue(*this, "client_max_body_size");
            if (this->_locationPath.empty()) {
                throw std::string("Error: location block doesn't have locationPath.");
            }
            if (!this->_return.empty()) {
                if (this->_return.size() != 2) {
                    throw std::string("Error: invalid number of arguments in location[return].");
                } else if (!Parser::isNumber(this->_return[0])) {
                    throw std::string("Error: invalid status code in location[return] directive.");
                }
            }
        }

        void inheritDirectives() {
            std::cout << "location root: " << this->dirMap["root"] << std::endl;
            if (this->dirMap["root"].empty()) {
                std::cout << "inherit root: " << this->dirMap["root"] << std::endl;
                this->dirMap["root"] = _inheritData.root;
            }
            if (this->index.empty()) {
                this->index = _inheritData.index;
            }
            if (this->dirMap["autoindex"].empty()) {
                this->dirMap["autoindex"] = _inheritData.autoindex;
            }
            if (this->error_page.empty()) {
                this->error_page = _inheritData.error_page;
            }
            if (this->dirMap["client_max_body_size"].empty()) {
                this->dirMap["client_max_body_size"] = _inheritData.clientMaxBodySize;
            }
        }

        void setBlock() {
            std::string buf = this->rawData;
            std::size_t pos = 0;
            while (buf[pos]) {
                std::string tmpLine = Parser::getIdentifier(buf, pos, "\n", false);
                if (Parser::sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = Parser::sideSpaceTrim(Parser::getIdentifier(tmpLine, tmpPos, " ", true));
                // std::cout << "identifier[location]: [" << tmpDir << "]" << std::endl;
                if (find(this->dirCase.begin(), this->dirCase.end(), tmpDir) == this->dirCase.end()) {
                    throw std::string("Error: " + tmpDir + " is not in block[location] list.");
                } else {
                    std::string tmpVal = Parser::sideSpaceTrim(Parser::getIdentifier(tmpLine, tmpPos, ";", true));
                    if (tmpDir == "index") {
                        this->index = Parser::getSplitBySpace(tmpVal);
                    } else if (tmpDir == "error_page") {
                        this->error_page = Parser::getSplitBySpace(tmpVal);
                    } else if (tmpDir == "allowed_method") {
                        this->allowed_method = Parser::getSplitBySpace(tmpVal);
                    } else if (tmpDir == "inner_proxy") {
                        this->inner_proxy = Parser::getSplitBySpace(tmpVal);
                    } else if (tmpDir == "return") {
                        this->_return = Parser::getSplitBySpace(tmpVal);
                    } else {
                        std::vector<std::string> tmpSplit = Parser::getSplitBySpace(tmpVal);
                        if (tmpSplit.size() != 1) {
                            throw std::string("Error: invalid number of arguments in location["+ tmpDir + " directive].");
                        }
                        this->dirMap[tmpDir] = tmpSplit[0];
                    }
                }
            }
        }
};

class ServerBlock : public NginxBlock{
    public:
        std::vector<std::string> dirCase;
        std::vector<std::string> index;
        std::vector<std::string> error_page;
        std::vector<class LocationBlock> location;

        ServerBlock() {}
        ServerBlock(std::string rawData) : NginxBlock(rawData) {
            setDirectiveTypes();
            setBlock();
            checkServerBlock();
        }

        void setDirectiveTypes() {
            this->dirCase.push_back("root");
            this->dirCase.push_back("index");
            this->dirCase.push_back("autoindex");
            this->dirCase.push_back("error_page");

            this->dirCase.push_back("listen");
            this->dirCase.push_back("server_name");
            this->dirCase.push_back("location");
            this->dirCase.push_back("client_max_body_size");
            this->dirCase.push_back("keepalive_timeout");
        }

        InheritData getInheritData() {
            InheritData inheritData;
            if (!this->dirMap["root"].empty()) {
                inheritData.root = this->dirMap["root"];
            }
            if (!this->dirMap["autoindex"].empty()) {
                inheritData.autoindex = this->dirMap["autoindex"];
            }
            if (!this->index.empty()) {
                inheritData.index = this->index;
            }
            if (!this->error_page.empty()) {
                inheritData.error_page = this->error_page;
            }
            if (!this->dirMap["client_max_body_size"].empty()) {
                inheritData.clientMaxBodySize = this->dirMap["client_max_body_size"];
            }
            return inheritData;
        }

        void setBlock() {
            std::string buf = this->rawData;
            std::size_t pos = 0;
            std::size_t blockPos = 0;
            // std::cout << "[DEBUG] rawData: " << this->rawData << std::endl;
            while (buf[pos]) {
                std::string tmpLine = Parser::getIdentifier(buf, pos, "\n", false);
                if (Parser::sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = Parser::getIdentifier(tmpLine, tmpPos, " ", true);
                // std::cout << "identifier[server]: [" << tmpDir << "]" << std::endl;
                if (find(this->dirCase.begin(), this->dirCase.end(), tmpDir) == this->dirCase.end()) {
                    throw std::string("Error: " + tmpDir + " is not in block[server] list.");
                } else if (tmpDir == "location") {
                    InheritData inheritData = getInheritData();
                    LocationBlock tmpLocationBlock(NginxParser::getBlockContent(buf, blockPos), Parser::sideSpaceTrim(Parser::getIdentifier(tmpLine, tmpPos, "{", true)), inheritData);
                    this->location.push_back(tmpLocationBlock);
                    pos = blockPos;
                } else {
                    std::string tmpVal = Parser::sideSpaceTrim(Parser::getIdentifier(tmpLine, tmpPos, ";", true));
                    if (tmpDir == "index") {
                        this->index = Parser::getSplitBySpace(tmpVal);
                    } else if (tmpDir == "error_page") {
                        this->error_page = Parser::getSplitBySpace(tmpVal);
                    } else {
                        std::vector<std::string> tmpSplit = Parser::getSplitBySpace(tmpVal);
                        if (tmpSplit.size() != 1) {
                            throw std::string("Error: invalid number of arguments in server["+ tmpDir + " directive].");
                        }
                        this->dirMap[tmpDir] = tmpSplit[0];
                    }
                }
            }
        }

        void checkServerBlock () {
            checkValidNumberValue(*this, "listen");
            checkValidNumberValue(*this, "client_max_body_size");
            checkValidNumberValue(*this, "keepalive_timeout");
            checkValidErrorPage(this->error_page);
            checkAutoindexValue(*this);
        }

};
class HttpBlock : public NginxBlock{
    public:
        std::vector<std::string> dirCase;
        std::vector<class ServerBlock> server;
        class TypesBlock types;
    
        HttpBlock() {}
        HttpBlock(std::string rawData) : NginxBlock(rawData) {
            setDirectiveTypes();
            setBlock();
        }

        void setDirectiveTypes() {
            this->dirCase.push_back("charset");
            this->dirCase.push_back("default_type");
            this->dirCase.push_back("keepalive_timeout");
            this->dirCase.push_back("sendfile");
            this->dirCase.push_back("types");
            this->dirCase.push_back("server");
        }

        // 1. 한 줄을 읽어옵니다.
        // 2. 한 줄에서 " " 기준으로 directive를 임시(tmpDir)로 읽어옵니다.
        // 3. 만약, tmpDir이 block 구조체에 포함된 멤버 변수라면 할당하고 그렇지 않으면 오류를 반환합니다.
        // 4. tmpDir이 block directive인 경우:
        //    - blockPos 다음 인덱스부터 가장 가까운 블록을 찾습니다.(blockPos는 getBlockContent에서 자동갱신)
        //    - block을 찾은 후에는 pos를 blockPos로 갱신합니다.
        // 5. tmpDir이 그냥 directive인 경우:
        //    - ";" 앞의 요소를 가지고 옵니다.
        //    - tmpDir을 구할 때 tmpPos가 이동해있으므로 directive 뒤의 " "와 ";" 사이의 값을 읽어오는 셈입니다.
        void setBlock() {
            std::size_t pos = 0;
            std::size_t blockPos = 0;
            while (this->rawData[pos]) {
                std::string tmpLine = Parser::getIdentifier(this->rawData, pos, "\n", false);
                if (Parser::sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = Parser::getIdentifier(tmpLine, tmpPos, " ", true);
                if (find(this->dirCase.begin(), this->dirCase.end(), tmpDir) == this->dirCase.end()) {
                    throw std::string("Error: " + tmpDir + " is not in block[http] list. HttpBlock::setHttpBlock");
                } else if (tmpDir == "types") {
                    TypesBlock tmpTypesBlock(NginxParser::getBlockContent(this->rawData, blockPos));
                    this->types = tmpTypesBlock;
                    pos = blockPos;
                } else if (tmpDir == "server") {
                    ServerBlock tmpServerBlock(NginxParser::getBlockContent(this->rawData, blockPos));
                    this->server.push_back(tmpServerBlock);
                    pos = blockPos;
                } else {
                    std::string tmpVal = Parser::sideSpaceTrim(Parser::getIdentifier(tmpLine, tmpPos, ";", true));
                    std::vector<std::string> tmpSplit = Parser::getSplitBySpace(tmpVal);
                    if (tmpSplit.size() != 1) {
                        throw std::string("Error: invalid number of arguments in http["+ tmpDir + " directive]. NginxConfig::setHttpBlock");
                    }
                    this->dirMap[tmpDir] = tmpSplit[0];
                }
            }
        }
};


// TODO: conf 파싱부분 수정
// - error_page 부분 제일 마지막에 파일명이 와야함


// NOTE: parsing 순서
// 1. directive 임시적으로 파싱 후, 해당하는 구조체에 포함되어 있는지 확인
// 2. 포함 된다면, 블록인지 아닌지 확인
//    -- 블록아님: ";" 이전 부분을 해당하는 directive의 값으로 설정
//    -- 블록: 3. 해당 블록을 위한 함수호출 전, rawData 세팅 (가장 큰 괄호 내부 데이터)
// 3. 해당 블록을 위한 함수를 호출
// 4. 
class NginxConfig : public NginxParser {
    public:
    public:
        class NoneBlock _none;
        class HttpBlock _http;

        NginxConfig(const std::string& fileName) : NginxParser(fileName) {
            std::size_t pos = 0;
            std::string identifier;

            while (_rawData[pos]) {
                std::string tmpLine = getIdentifier(_rawData, pos, "\n", false);
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }

                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ", true);
                std::size_t blockPos = 0;
                if (tmpDir == "http") {
                    HttpBlock tmpHttpBlock(NginxParser::getBlockContent(_rawData, blockPos));
                    _http = tmpHttpBlock;
                    pos = blockPos;
                } else {
                    std::string tmpVal = getIdentifier(tmpLine, tmpPos, ";", true);
                    if (tmpDir == "user") {
                        _none.user = sideSpaceTrim(tmpVal);
                    } else if (tmpDir == "worker_processes") {
                        _none.worker_processes = sideSpaceTrim(tmpVal);
                    } else {
                        throw std::string("Error: " + tmpDir + " is not in block[none] list. NginxConfig::NginxConfig");
                    }
                }
            }
        }
};
}
#endif