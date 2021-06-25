#ifndef NGINXCONFIG_HPP
#define NGINXCONFIG_HPP

#include "NginxParser.hpp"

class NginxConfig : public NginxParser {
    private:
        struct nginxBlock {
            std::string rawData;

        };
        struct noneBlock : nginxBlock {
            std::string user;
            std::string worker_processes;
        };
        struct typesBlock : nginxBlock{
            std::string html;
            std::string css;
            std::string xml;
        };
        struct locationBlock : nginxBlock {
            std::string locationPath;
            std::vector<std::string> locationReturn;
        };
        struct serverBlock : nginxBlock{
            std::string listen;
            std::string server_name;
            struct locationBlock location[2];
        };
        struct httpBlock : nginxBlock{
            std::string charset;
            std::string include;
            std::string default_type;
            std::string keeplive_timeout;

            struct serverBlock server[2];
            struct typesBlock types;
        };

    public:
        struct noneBlock _none;
        struct httpBlock _http;

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
                }
                lineBegPos = lineEndPos;
            }

            std::cout << "_none::user -> " << _none.user << std::endl;
            std::cout << "_none::worker_processes -> " << _none.worker_processes << std::endl;
            std::cout << "_http::charset -> " << _http.charset << std::endl;
            std::cout << "_http::include -> " << _http.include << std::endl;
            std::cout << "_http::default_type -> " << _http.default_type << std::endl;
            std::cout << "_http::keepalive_timeout -> " << _http.keeplive_timeout << std::endl;
            std::cout << "_http::server::listen -> " << _http.server[0].listen << std::endl;
            std::cout << "_http::server::server_name -> " << _http.server[0].server_name << std::endl;
            std::cout << "_http::server::location::path -> " << _http.server[0].location[0].locationPath << std::endl;
            std::cout << "_http::types::html -> " << _http.types.html << std::endl;
            std::cout << "_http::types::css -> " << _http.types.css << std::endl;
        }

    public:

        void setTypesBlock(struct typesBlock& block) {
            std::size_t pos = 0;

            std::cout << "TypesBlock: [" << block.rawData << "]" << std::endl;

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
                } 
                // std::cout << "identifier[types]: " << tmpDir << std::endl;
            }
        }

        void setLocationBlock(struct locationBlock& block) {
            std::size_t pos = 0;

            std::string buf = block.rawData;
            while (buf[pos]) {
                // line 위치 기록중
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (tmpLine.empty()) {
                    continue ;
                }
                std::size_t delimitPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
                if (tmpDir == "return") {
                    block.locationReturn.push_back(leftSpaceTrim(getIdentifier(tmpLine, delimitPos, " ")));
                    block.locationReturn.push_back(leftSpaceTrim(getIdentifier(tmpLine, delimitPos, ";")));
                } 
                // std::cout << "identifier[location]: " << tmpDir << std::endl;
            }
        }

        // 기본적으로, new line의 위치를 기록하며 따라가다가, block directive 때는 건너뜀
        void setServerBlock(struct serverBlock& block) {
            std::size_t pos = 0;

            std::string buf = block.rawData;
            std::size_t lineBegPos = 0;
            std::size_t lineEndPos = 0;
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }
                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ");
                if (tmpDir == "listen") {
                    block.listen = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "server_name") {
                    block.server_name = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "location") {
                    lineEndPos = pos;
                    block.location[0].locationPath = getIdentifier(tmpLine, tmpPos, " ");
                    block.location[0].rawData = getBlockContent(buf, lineBegPos);
                    pos = lineBegPos;
                    setLocationBlock(block.location[0]);
                }
                lineBegPos = lineEndPos;
                // std::cout << "identifier[server]: " << tmpDir << std::endl;
            }
            // std::cout << "server.listen: " << ret.listen << std::endl;
            // std::cout << "server.server_name: " << ret.server_name << std::endl;
        }

        // 독립적으로 사용됨. pos를 받고 진행 상황을 기록할 필요 없음
        void setHttpBlock(struct httpBlock& block) {
            std::size_t pos = 0;

            std::string buf = block.rawData;
            std::size_t lineBegPos = 0;
            std::size_t lineEndPos = 0;
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                std::cout << "pos[HTTP]: " << pos << std::endl;
                if (sideSpaceTrim(tmpLine).empty()) {
                    continue ;
                }

                std::size_t tmpPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, tmpPos, " ");
                std::cout << "identifier[http]: [" << tmpDir << "]" << std::endl;
                if (tmpDir == "charset") {
                    block.charset = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "include") {
                    block.include = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "default_type") {
                    block.default_type = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "keepalive_timeout") {
                    block.keeplive_timeout = sideSpaceTrim(getIdentifier(tmpLine, tmpPos, ";"));
                } else if (tmpDir == "server") {
                    lineEndPos = pos;
                    block.server[0].rawData = getBlockContent(buf, lineBegPos);
                    pos = lineBegPos;
                    setServerBlock(block.server[0]);
                    return ;
                } else if (tmpDir == "types") {
                    lineEndPos = pos;
                    block.types.rawData = getBlockContent(buf, lineBegPos);
                    pos = lineBegPos;
                    setTypesBlock(block.types);
                } else {
                    throw std::string("Error: not in block list.");
                }
                lineBegPos = pos;
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
    }
}
#endif