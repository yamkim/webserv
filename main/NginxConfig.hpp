#ifndef NGINXCONFIG_HPP
#define NGINXCONFIG_HPP

#include "NginxParser.hpp"

class NginxConfig : public NginxParser {
    public:
        struct NginxBlock {
            std::string rawData;
        };
        struct NoneBlock : NginxBlock {
            std::string user;
            std::string worker_processes;
        };
        struct TypesBlock : NginxBlock{
            std::string html;
            std::string css;
            std::string xml;
        };
        struct LocationBlock : NginxBlock {
            // FIXME: yekim: cgi 파라미터 관련 요소 (cgi_pass) 추가
            std::string locationPath;
            std::string try_files;
            std::string deny;
            std::string locationReturn;
        };
        struct ServerBlock : NginxBlock{
            std::string listen;
            std::string server_name;
            std::string root;
            std::string index;
            struct LocationBlock location[10];
        };
        struct HttpBlock : NginxBlock{
            std::string charset;
            std::string include;
            std::string default_type;
            std::string keeplive_timeout;
            std::string sendfile;

            struct ServerBlock server[2];
            struct TypesBlock types;
        };

    public:
        struct NoneBlock _none;
        struct HttpBlock _http;

        NginxConfig(const std::string& fileName) : NginxParser(fileName) {
            std::size_t pos = 0;
            std::string identifier;

            std::size_t lineBegPos = 0;
            std::size_t lineEndPos = 0;
            while (_rawData[pos]) {
                std::string tmpLine = getIdentifier(_rawData, pos, "\n");
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }

                // 일단 한 번 쪼개보고, identifer가 블록인지 블록이 아닌지 판단
                // 블록이면, 블록 형태의 structure, 아니면 none 블록 형태의 structure로
                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ");
                if (tmpDir == "http") {
                    lineEndPos = pos;
                    _http.rawData = getBlockContent(_rawData, lineBegPos);
                    setHttpBlock(_http);
                    pos = lineBegPos; // 블록 이후의 pos
                } else if (tmpDir == "user") {
                    _none.user = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "worker_processes") {
                    _none.worker_processes = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else {
                    throw std::string("Error: " + tmpDir + " is not in block[none] list.");
                }
                lineBegPos = lineEndPos;
            }

            std::cout << "_none::user -> " << _none.user << std::endl;
            std::cout << "_none::worker_processes -> " << _none.worker_processes << std::endl;
            std::cout << "_http::charset -> " << _http.charset << std::endl;
            std::cout << "_http::include -> " << _http.include << std::endl;
            std::cout << "_http::default_type -> " << _http.default_type << std::endl;
            std::cout << "_http::keepalive_timeout -> " << _http.keeplive_timeout << std::endl;
            std::cout << "_http::server[0]::listen -> " << _http.server[0].listen << std::endl;
            std::cout << "_http::server[0]::server_name -> " << _http.server[0].server_name << std::endl;
            std::cout << "_http::server[0]::location[0]::path -> " << _http.server[0].location[0].locationPath << std::endl;
            std::cout << "_http::server[1]::listen -> " << _http.server[1].listen << std::endl;
            std::cout << "_http::server[1]::root -> " << _http.server[1].root << std::endl;
            std::cout << "_http::server[1]::index -> " << _http.server[1].index << std::endl;
            std::cout << "_http::server[1]::location[0]::path -> " << _http.server[1].location[0].locationPath << std::endl;
            std::cout << "_http::server[1]::location[0]::try_files -> " << _http.server[1].location[0].try_files << std::endl;
            std::cout << "_http::server[1]::location[1]::path -> " << _http.server[1].location[1].locationPath << std::endl;
            std::cout << "_http::server[1]::location[2]::path -> " << _http.server[1].location[2].locationPath << std::endl;
            std::cout << "_http::types::html -> " << _http.types.html << std::endl;
            std::cout << "_http::types::css -> " << _http.types.css << std::endl;
        }

    public:

        void setTypesBlock(struct TypesBlock& block) {
            std::size_t pos = 0;

            std::string buf = block.rawData;
            while (buf[pos]) {
                // line 위치 기록중
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ");
                if (tmpDir == "text/html") {
                    block.html = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "text/css") {
                    block.css = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "text/xml") {
                    block.xml = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else {
                    throw std::string("Error: " + tmpDir + " is not in block[types] list.");
                }
                // std::cout << "identifier[types]: " << tmpDir << std::endl;
            }
        }

        void setLocationBlock(struct LocationBlock& block) {
            std::string buf = block.rawData;

            std::size_t pos = 0;
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, " "));
                // std::cout << "identifier[location]: [" << tmpDir << "]" << std::endl;
                if (tmpDir == "return") {
                    block.locationReturn = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                    std::cout << block.locationReturn << std::endl;
                } else if (tmpDir == "try_files") {
                    block.try_files = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";")); 
                } else if (tmpDir == "deny") {
                    block.deny = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";")); 
                } else {
                    throw std::string("Error: " + tmpDir + " is not in block[location] list.");
                }
            }
        }

        void setServerBlock(struct ServerBlock& block) {
            std::string buf = block.rawData;

            std::size_t pos = 0;
            std::size_t blockPos = 0;
            std::size_t locationBlockIdx = 0;
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ");
                // std::cout << "identifier[server]: [" << tmpDir << "]" << std::endl;
                if (tmpDir == "listen") {
                    block.listen = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "server_name") {
                    block.server_name = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "root") {
                    block.root = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "index") {
                    block.index = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "location") {
                    block.location[locationBlockIdx].locationPath = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, "{"));
                    block.location[locationBlockIdx].rawData = getBlockContent(buf, blockPos);
                    setLocationBlock(block.location[locationBlockIdx]);
                    locationBlockIdx++;
                    pos = blockPos;
                } else {
                    throw std::string("Error: " + tmpDir + " is not in block[server] list.");
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
            std::string buf = block.rawData;

            std::size_t pos = 0;
            std::size_t blockPos = 0;
            std::size_t serverBlockIdx = 0;
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ");
                // std::cout << "identifier[http]: [" << tmpDir << "]" << std::endl;
                if (tmpDir == "charset") {
                    block.charset = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "include") {
                    block.include = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "default_type") {
                    block.default_type = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "keepalive_timeout") {
                    block.keeplive_timeout = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "sendfile") {
                    block.sendfile = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "server") {
                    block.server[serverBlockIdx].rawData = getBlockContent(buf, blockPos);
                    setServerBlock(block.server[serverBlockIdx]);
                    serverBlockIdx++;
                    pos = blockPos;
                } else if (tmpDir == "types") {
                    block.types.rawData = getBlockContent(buf, blockPos);
                    setTypesBlock(block.types);
                    pos = blockPos;
                } else {
                    throw std::string("Error: " + tmpDir + " is not in block[http] list.");
                }
            }
        }
};
#endif

#if 0
void setHttp(struct nginxBlock* block, std::string tmpDir, std::string tmpLine, std::size_t &pos) {
    struct httpBlock* tmpHttpBlock = (reinterpret_cast<struct httpBlock*>(block));
    std::string buf = tmpHttpBlock->rawData;
    std::size_t delimitPos = 0;
    if (tmpDir == "charset") {
        tmpHttpBlock->charset = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
    } else if (tmpDir == "include") {
        tmpHttpBlock->include = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
    } else if (tmpDir == "default_type") {
        tmpHttpBlock->default_type = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
    } else if (tmpDir == "server") {
        std::string blockContent = sideSpaceTrim(getBlockContent(buf, pos));
        setBlock(&(tmpHttpBlock->server[0]), NULL);
    }
}

void setBlock(nginxBlock* block, void (*fp)(void*, std::string, std::string, std::size_t&)) {
    std::size_t pos = 0;

    if (fp == nullptr) {
        throw std::string("Error: fp Error");
    }
    std::string buf = block->rawData;
    while (buf[pos]) {
        std::string tmpLine = getIdentifier(buf, pos, "\n");
        if (tmpLine.empty()) {
            continue ;
        }
        std::size_t delimitPos = 0;
        std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
        fp(block, tmpDir, tmpLine, pos);
        // std::cout << "identifier[http]: " << tmpDir << std::endl;
        //setBlock(block, setHttp);
    // }
}
#endif