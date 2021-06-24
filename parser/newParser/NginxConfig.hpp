#ifndef NGINXCONFIG_HPP
#define NGINXCONFIG_HPP

#include "NginxParser.hpp"

class NginxConfig : public NginxParser {
    private:
        struct noneBlock {
            std::string user;
            std::string worker_processes;
        };
        struct locationBlock {
            std::string locationPath;
            std::vector<std::string> locationReturn;
        };
        struct serverBlock {
            std::string listen;
            std::string server_name;
            struct locationBlock location[2];
        };
        struct httpBlock {
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
                    std::string blockContent = getBlockContent(_rawData, delimitPos);
                    _http = setHttpBlock(blockContent);
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
        struct locationBlock setLocationBlock(const std::string& buf, std::string locationPath) {
            std::size_t pos = 0;
            struct locationBlock ret;

            ret.locationPath = locationPath;
            while (buf[pos]) {
                // line 위치 기록중
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (tmpLine.empty()) {
                    continue ;
                }
                std::size_t delimitPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
                if (tmpDir == "listen") {
                    ret.locationReturn.push_back(leftSpaceTrim(getIdentifier(tmpLine, delimitPos, " ")));
                    ret.locationReturn.push_back(leftSpaceTrim(getIdentifier(tmpLine, delimitPos, ";")));
                } 
                // std::cout << "identifier[location]: " << tmpDir << std::endl;
            }
            // std::cout << "location.locationPath: " << ret.locationPath << std::endl;
            return ret;
        }

        // 기본적으로, new line의 위치를 기록하며 따라가다가, block directive 때는 건너뜀
        struct serverBlock setServerBlock(const std::string& buf) {
            std::size_t pos = 0;
            struct serverBlock ret;

            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (tmpLine.empty()) {
                    continue ;
                }
                std::size_t delimitPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
                if (tmpDir == "listen") {
                    ret.listen = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "server_name") {
                    ret.server_name = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "location") {
                    std::string locationPath = getIdentifier(tmpLine, delimitPos, " ");
                    std::string blockContent = getBlockContent(buf, delimitPos);
                    pos = delimitPos;

                    ret.location[0] = setLocationBlock(blockContent, locationPath);
                }
                // std::cout << "identifier[server]: " << tmpDir << std::endl;
            }
            // std::cout << "server.listen: " << ret.listen << std::endl;
            // std::cout << "server.server_name: " << ret.server_name << std::endl;
            return ret;
        }

        // 독립적으로 사용됨. pos를 받고 진행 상황을 기록할 필요 없음
        struct httpBlock setHttpBlock(const std::string& buf) {
            struct httpBlock ret;
            std::size_t pos = 0;

            while (buf[pos]) {
                std::string tmpLine = getIdentifier(buf, pos, "\n");
                if (tmpLine.empty()) {
                    continue ;
                }
                std::size_t delimitPos = 0;
                std::string tmpDir = getIdentifier(tmpLine, delimitPos, " ");
                if (tmpDir == "charset") {
                    ret.charset = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "include") {
                    ret.include = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "default_type") {
                    ret.default_type = sideSpaceTrim(getIdentifier(tmpLine, delimitPos, ";"));
                } else if (tmpDir == "server") {
                    std::string blockContent = sideSpaceTrim(getBlockContent(buf, delimitPos));
                    pos = delimitPos;
                    ret.server[0] = setServerBlock(blockContent);
                }
                // std::cout << "identifier[http]: " << tmpDir << std::endl;
            }

            // std::cout << "http charset: " << ret.charset << std::endl;
            // std::cout << "http include: " << ret.include << std::endl;
            return ret;
        }
};
#endif