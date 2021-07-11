#ifndef NGINXCONFIG_HPP
#define NGINXCONFIG_HPP

#include "NginxParser.hpp"

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
        struct NginxBlock {
            std::string rawData;
            std::map<std::string, std::string> dirMap;

            NginxBlock() {};
            NginxBlock(std::string rawData) : rawData(rawData) {};
        };
        struct NoneBlock : NginxBlock {
            std::vector<std::string> dirCase;
            std::string user;
            std::string worker_processes;
        };
        struct TypesBlock : NginxBlock {
            std::map<std::string, std::string> typeMap;
            TypesBlock() {}
            TypesBlock(std::string rawData) : NginxBlock(rawData) {}
        };
        struct LocationBlock : NginxBlock {
            std::vector<std::string> dirCase;
            // FIXME: yekim: cgi 파라미터 관련 요소 (cgi_pass) 추가
            std::string locationPath;
            std::vector<std::string> try_files;
            std::vector<std::string> _return;
            std::vector<std::string> deny;
            std::vector<std::string> index;
            std::vector<std::string> error_page;
            LocationBlock() {}
            LocationBlock(std::string rawData) : NginxBlock(rawData) {}
        };
        struct ServerBlock : NginxBlock{
            std::vector<std::string> dirCase;
            std::vector<std::string> index;
            std::vector<std::string> error_page;
            std::vector<struct LocationBlock> location;

            ServerBlock() {}
            ServerBlock(std::string rawData) : NginxBlock(rawData) {}
        };
        struct HttpBlock : NginxBlock{
            std::vector<std::string> dirCase;
            std::vector<struct ServerBlock> server;
            struct TypesBlock types;
        };

    public:
        struct NoneBlock _none;
        struct HttpBlock _http;

        void checkValidErrorPage(const std::vector<std::string>& errorPage) {
            std::vector<std::string>::const_iterator iter;
            if (!errorPage.empty()) {
                for (iter = errorPage.begin(); iter != errorPage.end() - 1; ++iter) {
                    if (!isNumber(*iter)) {
                        throw std::string("Error: invalid value " + *iter + " in error_page directive.");
                    }
                }
            }
        }

        void checkValidNumberValue(NginxBlock& block, std::string directive) {
            if (!block.dirMap[directive].empty() && !isNumber(block.dirMap[directive])) {
                throw std::string("Error: invalid value in " + directive + " directive.");
            }
        }

        void checkLocationBlock(LocationBlock& block) {
            if (block.locationPath.empty()) {
                throw std::string("Error: location block doesn't have locationPath.");
            }
            if (!block.error_page.empty()) {
                checkValidErrorPage(block.error_page);
            }

        }

        void checkServerBlock (ServerBlock& block) {
            checkValidNumberValue(block, "listen");
            checkValidNumberValue(block, "client_max_body_size");
            checkValidNumberValue(block, "keepalive_timeout");
            checkValidErrorPage(block.error_page);
            if (!block.dirMap["autoindex"].empty() && !(block.dirMap["autoindex"] == "on"
                  || block.dirMap["autoindex"] == "off")) {
                throw std::string("Error: invalid argument for autoindex directive.");
            }
            for (std::size_t i = 0; i < block.location.size(); ++i) {
                checkLocationBlock(block.location[i]);
            }
        }

        NginxConfig(const std::string& fileName) : NginxParser(fileName) {
            std::size_t pos = 0;
            std::string identifier;

            std::size_t lineBegPos = 0;
            std::size_t lineEndPos = 0;
            while (_rawData[pos]) {
                std::string tmpLine = getIdentifier(_rawData, pos, "\n", false);
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }

                // 일단 한 번 쪼개보고, identifer가 블록인지 블록이 아닌지 판단
                // 블록이면, 블록 형태의 structure, 아니면 none 블록 형태의 structure로
                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ", true);
                if (tmpDir == "http") {
                    lineEndPos = pos;
                    _http.rawData = getBlockContent(_rawData, lineBegPos);
                    setHttpBlock(_http);
                    pos = lineBegPos; // 블록 이후의 pos
                } else {
                    std::string tmpVal = getIdentifier(tmpLine, tmpPos, ";", true);
                    if (tmpDir == "user") {
                        _none.user = sideSpaceTrim(tmpVal);
                    } else if (tmpDir == "worker_processes") {
                        _none.worker_processes = sideSpaceTrim(tmpVal);
                    } else {
                        throw std::string("Error: " + tmpDir + " is not in block[none] list.");
                    }
                }
                lineBegPos = lineEndPos;
            }
            checkValidNumberValue(_none, "keepalive_timeout");
            for (std::size_t i = 0; i < _http.server.size(); ++i) {
                checkServerBlock(_http.server[i]);
            }
        }

    public:

        struct TypesBlock setTypesBlock(struct TypesBlock& block) {
            std::string buf = block.rawData;
            std::size_t pos = 0;
            // TODO: getIdentifier를 세분화해서 getLine하고 나누어야할듯.
            while (pos < buf.size()) {
                std::string tmpLine = getIdentifier(buf, pos, "\n", false);
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ", true);
                // ";" 이전까지 파싱하고, " "로 구분하므로, 마지막 요소는 그냥 사용
                std::string value = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";", true));
                std::cout << "[DEBUG] value in tmpLine: [" << value << "]" << std::endl;
                setTypeMap(block.typeMap, tmpDir, value);
            }
            return block;
        }

        void setLocationBlock(struct LocationBlock& block) {
            block.dirCase.push_back("return");
            block.dirCase.push_back("try_files");
            block.dirCase.push_back("deny");
            block.dirCase.push_back("autoindex");
            block.dirCase.push_back("index");
            block.dirCase.push_back("error_page");
            block.dirCase.push_back("cgi_pass");

            std::string buf = block.rawData;
            std::size_t pos = 0;
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n", false);
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, " ", true));
                // std::cout << "identifier[location]: [" << tmpDir << "]" << std::endl;
                if (find(block.dirCase.begin(), block.dirCase.end(), tmpDir) == block.dirCase.end()) {
                    throw std::string("Error: " + tmpDir + " is not in block[location] list.");
                } else {
                    std::string tmpVal = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";", true));
                    if (tmpDir == "index") {
                        block.index = getSplitBySpace(tmpVal);
                    } else if (tmpDir == "error_page") {
                        block.error_page = getSplitBySpace(tmpVal);
                    } else if (tmpDir == "try_files") {
                        block.try_files = getSplitBySpace(tmpVal);
                    } else if (tmpDir == "return") {
                        block._return = getSplitBySpace(tmpVal);
                        if (block._return.size() != 2) {
                            throw std::string("Error: invalid number of arguments in location["+ tmpDir + " directive].");
                        } else if (!isNumber(block._return[0])) {
                            throw std::string("Error: invalid status code in "+ tmpDir + " directive.");
                        }
                    } else {
                        std::vector<std::string> tmpSplit = getSplitBySpace(tmpVal);
                        if (tmpSplit.size() != 1) {
                            throw std::string("Error: invalid number of arguments in location["+ tmpDir + " directive].");
                        }
                        block.dirMap[tmpDir] = tmpSplit[0];
                    }
                }
            }
        }

        void setServerBlock(struct ServerBlock& block) {
            block.dirCase.push_back("listen");
            block.dirCase.push_back("server_name");
            block.dirCase.push_back("root");
            block.dirCase.push_back("index");
            block.dirCase.push_back("location");
            block.dirCase.push_back("autoindex");
            block.dirCase.push_back("error_page");
            block.dirCase.push_back("client_max_body_size");
            block.dirCase.push_back("keepalive_timeout");

            std::string buf = block.rawData;
            std::size_t pos = 0;
            std::size_t blockPos = 0;
            std::cout << "[DEBUG] rawData: " << block.rawData << std::endl;
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n", false);
                std::cout << "[DEBUG] tmpLine: [" << tmpLine << "]" << std::endl;
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ", true);
                // std::cout << "identifier[server]: [" << tmpDir << "]" << std::endl;
                if (find(block.dirCase.begin(), block.dirCase.end(), tmpDir) == block.dirCase.end()) {
                    throw std::string("Error: " + tmpDir + " is not in block[server] list.");
                } else if (tmpDir == "location") {
                    LocationBlock tmpLocationBlock(getBlockContent(buf, blockPos));
                    tmpLocationBlock.locationPath = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, "{", true));
                    setLocationBlock(tmpLocationBlock);
                    block.location.push_back(tmpLocationBlock);
                    pos = blockPos;
                } else {
                    std::string tmpVal = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";", true));
                    if (tmpDir == "index") {
                        block.index = getSplitBySpace(tmpVal);
                    } else if (tmpDir == "error_page") {
                        block.error_page = getSplitBySpace(tmpVal);
                    } else {
                        std::vector<std::string> tmpSplit = getSplitBySpace(tmpVal);
                        if (tmpSplit.size() != 1) {
                            throw std::string("Error: invalid number of arguments in server["+ tmpDir + " directive].");
                        }
                        block.dirMap[tmpDir] = tmpSplit[0];
                    }
                }
            }
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
        void setHttpBlock(struct HttpBlock& block) {
            block.dirCase.push_back("charset");
            block.dirCase.push_back("default_type");
            block.dirCase.push_back("keepalive_timeout");
            block.dirCase.push_back("sendfile");
            block.dirCase.push_back("types");
            block.dirCase.push_back("server");

            std::size_t pos = 0;
            std::size_t blockPos = 0;
            while (block.rawData[pos]) {
                std::string tmpLine = getIdentifier(block.rawData, pos, "\n", false);
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::cout << "[DEBUG] tmpLine: " << tmpLine << std::endl;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ", true);
                if (find(block.dirCase.begin(), block.dirCase.end(), tmpDir) == block.dirCase.end()) {
                    throw std::string("Error: " + tmpDir + " is not in block[server] list.");
                } else if (tmpDir == "types") {
                    TypesBlock tmpTypesBlock(getBlockContent(block.rawData, blockPos));
                    block.types = setTypesBlock(tmpTypesBlock);
                    pos = blockPos;
                } else if (tmpDir == "server") {
                    ServerBlock tmpServerBlock(getBlockContent(block.rawData, blockPos));
                    setServerBlock(tmpServerBlock);
                    block.server.push_back(tmpServerBlock);
                    pos = blockPos;
                } else {
                    std::string tmpVal = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";", false));
                    std::vector<std::string> tmpSplit = getSplitBySpace(tmpVal);
                    if (tmpSplit.size() != 1) {
                        throw std::string("Error: invalid number of arguments in http["+ tmpDir + " directive]. NginxConfig::setHttpBlock");
                    }
                    block.dirMap[tmpDir] = tmpSplit[0];
                }
            }
        }
};
#endif