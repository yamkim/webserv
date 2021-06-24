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

            struct serverBlock server[2];
        };
    public:
        struct noneBlock _none;
        struct httpBlock _http;

        NginxConfig(const std::string& fileName) : NginxParser(fileName) {
            std::size_t pos = 0;
            std::string identifier;

            while (_rawData[pos]) {
                std::string tmpLine = getIdentifier(_rawData, pos, "\n");
                if (tmpLine.empty()) {
                    continue ;
                }

                // 일단 한 번 쪼개보고, identifer가 블록인지 블록이 아닌지 판단
                // 블록이면, 블록 형태의 structure, 아니면 none 블록 형태의 structure로
                std::size_t delimitPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
                if (tmpDir == "http") {
                    _http.rawData = getBlockContent(_rawData, delimitPos);
                    setHttpBlock(_http);
                    pos = delimitPos; // 블록 이후의 pos
                } else if (tmpDir == "user") {
                    _none.user = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "worker_processes") {
                    _none.worker_processes = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                }
            }

            std::cout << "_none::user -> " << _none.user << std::endl;
            std::cout << "_none::worker_processes -> " << _none.worker_processes << std::endl;
            std::cout << "_http::charset -> " << _http.charset << std::endl;
            std::cout << "_http::include -> " << _http.include << std::endl;
            std::cout << "_http::default_type -> " << _http.default_type << std::endl;
            std::cout << "_http::server::listen -> " << _http.server[0].listen << std::endl;
            std::cout << "_http::server::server_name -> " << _http.server[0].server_name << std::endl;
            std::cout << "_http::server::location::path -> " << _http.server[0].location[0].locationPath << std::endl;
        }

    public:

        // void setHttp(void* block, std::string tmpDir, std::string tmpLine, std::size_t &pos) {
        //     struct httpBlock* tmpHttpBlock = (reinterpret_cast<struct httpBlock*>(block));
        //     std::string buf = tmpHttpBlock->rawData;
        //     std::size_t delimitPos = 0;
        //     if (tmpDir == "charset") {
        //         tmpHttpBlock->charset = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
        //     } else if (tmpDir == "include") {
        //         tmpHttpBlock->include = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
        //     } else if (tmpDir == "default_type") {
        //         tmpHttpBlock->default_type = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
        //     } else if (tmpDir == "server") {
        //         std::string blockContent = sideSpaceTrim(getBlockContent(buf, pos));
        //         setBlock(&(tmpHttpBlock->server[0]), NULL);
        //     }
        // }

        // void setBlock(nginxBlock* block, void* fp()) {
        //     std::size_t pos = 0;

        //     if (fp == nullptr) {
        //         throw std::string("Error: fp Error");
        //     }
        //     std::string buf = block->rawData;
        //     while (buf[pos]) {
        //         std::string tmpLine = getIdentifier(buf, pos, "\n");
        //         if (tmpLine.empty()) {
        //             continue ;
        //         }
        //         std::size_t delimitPos = 0;
        //         std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
        //         // std::cout << "identifier[http]: " << tmpDir << std::endl;
        //         setBlock(block, tmpDir, tmpLine, pos);
        //     }
        // }

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
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (tmpLine.empty()) {
                    continue ;
                }
                std::size_t delimitPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
                if (tmpDir == "listen") {
                    block.listen = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "server_name") {
                    block.server_name = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "location") {
                    block.location[0].locationPath = getIdentifier(tmpLine, delimitPos, " ");
                    block.location[0].rawData = getBlockContent(buf, delimitPos);
                    pos = delimitPos;

                    setLocationBlock(block.location[0]);
                }
                // std::cout << "identifier[server]: " << tmpDir << std::endl;
            }
            // std::cout << "server.listen: " << ret.listen << std::endl;
            // std::cout << "server.server_name: " << ret.server_name << std::endl;
        }

        // 독립적으로 사용됨. pos를 받고 진행 상황을 기록할 필요 없음
        void setHttpBlock(struct httpBlock& block) {
            std::size_t pos = 0;

            std::string buf = block.rawData;
            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (tmpLine.empty()) {
                    continue ;
                }
                std::size_t delimitPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
                if (tmpDir == "charset") {
                    block.charset = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "include") {
                    block.include = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "default_type") {
                    block.default_type = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "server") {
                    block.server[0].rawData = sideSpaceTrim(getBlockContent(buf, delimitPos));
                    pos = delimitPos;

                    setServerBlock(block.server[0]);
                }
                // std::cout << "identifier[http]: " << tmpDir << std::endl;
            }

            // std::cout << "http charset: " << ret.charset << std::endl;
            // std::cout << "http include: " << ret.include << std::endl;
        }
};
#endif