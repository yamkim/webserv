#ifndef NGINXCONFIG_HPP
#define NGINXCONFIG_HPP

#include <vector>
#include <string>

class NginxConfig {
private:
        std::vector<std::string> noneBlockDirList;
        std::vector<std::string> eventsBlockDirList;
        std::vector<std::string> httpBlockDirList;
        std::vector<std::string> upstreamBlockDirList;
        std::vector<std::string> serverBlockDirList;
public:
    NginxConfig(){
        this->noneBlockDirList.push_back("user");
        this->noneBlockDirList.push_back("worker_processes");

        this->eventsBlockDirList.push_back("worker_connections");
        
        this->httpBlockDirList.push_back("charset");
        this->httpBlockDirList.push_back("include");
        this->httpBlockDirList.push_back("default_type");

        this->upstreamBlockDirList.push_back("server");
        this->upstreamBlockDirList.push_back("keepalive");

        this->serverBlockDirList.push_back("listen");
        this->serverBlockDirList.push_back("server_name");
        this->serverBlockDirList.push_back("include");
        this->serverBlockDirList.push_back("location");
    };
    virtual ~NginxConfig(){};

    struct NoneBlock {
	std::string user;
       std::string worker_processes;
    };
    struct EventsBlock {
        std::string worker_connections;
    }; 
    struct HttpBlock {
        std::string charset;
        std::string include;
        std::string default_type;
    };
    struct UpstreamBlock {
        std::string server;
        std::string keepalive;
    };
    struct ServerBlock {
        std::string listen;
        std::string server_name;
        std::string include;
        std::string proxy_pass;
        std::string location;
    };

    struct NoneBlock noneBlock;
    struct EventsBlock eventsBlock;
    struct HttpBlock httpBlock;
    struct UpstreamBlock upstreamBlock;
    struct ServerBlock serverBlock;

    bool isBlockDirective(const std::string& block, const std::string& directive) const {
        if (block == "none") {
            if (find(this->noneBlockDirList.begin(), this->noneBlockDirList.end(), directive) != this->noneBlockDirList.end()) {
                return true;
            }
        } else if (block == "events") {
            if (find(this->eventsBlockDirList.begin(), this->eventsBlockDirList.end(), directive) != this->eventsBlockDirList.end()) {
                return true;
            }
        } else if (block == "http") {
            if (find(this->httpBlockDirList.begin(), this->httpBlockDirList.end(), directive) != this->httpBlockDirList.end()) {
                return true;
            }
        } else if (block == "upstream") {
            if (find(this->upstreamBlockDirList.begin(), this->upstreamBlockDirList.end(), directive) != this->upstreamBlockDirList.end()) {
                return true;
            }
        } else if (block == "server") {
            if (find(this->serverBlockDirList.begin(), this->serverBlockDirList.end(), directive) != this->serverBlockDirList.end()) {
                return true;
            }
        }
        return false;    
    }

    std::string getDirectiveValue(const std::string& block, const std::string& directive) const {
        if (!this->isBlockDirective(block, directive))
            throw "Invalid directive(getDirectiveValue): " + directive;
        if (block == "none") {
            if (directive == "user") {
                return (noneBlock.user);
            } else if (directive == "worker_processes") {
                return (noneBlock.worker_processes);
            }
        } else if (block == "events") {
            if (directive == "worker_connections") {
                return (eventsBlock.worker_connections);
            }
        } else if (block == "http") {
            if (directive == "charset") {
                return (httpBlock.charset);
            } else if (directive == "include") {
                return (httpBlock.include);
            } else if (directive == "default_type") {
                return (httpBlock.default_type);
            }
        } else if (block == "upstream") {
            if (directive == "server") {
                return (upstreamBlock.server);
            } else if (directive == "keepalive") {
                return (upstreamBlock.keepalive);
            }
        } else if (block == "server") {
            if (directive == "listen") {
                return (serverBlock.listen);
            } else if (directive == "server_name") {
                return (serverBlock.server_name);
            } else if (directive == "include") {
                return (serverBlock.include);
            } else if (directive == "location") {
                return (serverBlock.location);
            }
        }
        return ("");
    }

    void setDirectiveValue(const std::string& block, const std::string& directive, const std::string& value) {
        if (!this->isBlockDirective(block, directive))
            throw "Invalid directive(getDirectiveValue): " + directive;
        if (block == "none") {
            if (directive == "user") {
                this->noneBlock.user = value;
                return ;
            } else if (directive == "worker_processes") {
                this->noneBlock.worker_processes = value;
                return ;
            }
        } else if (block == "events") {
            if (directive == "worker_connections") {
                this->eventsBlock.worker_connections = value;
                return ;
            }
        } else if (block == "http") {
            if (directive == "charset") {
                this->httpBlock.charset = value;
                return ;
            } else if (directive == "include") {
                this->httpBlock.include = value;
                return ;
            } else if (directive == "default_type") {
                this->httpBlock.default_type = value;
                return ;
            }
        } else if (block == "upstream") {
            if (directive == "server") {
                this->upstreamBlock.server = value;
                return ;
            } else if (directive == "keepalive") {
                this->upstreamBlock.keepalive = value;
                return ;
            }
        } else if (block == "server") {
            if (directive == "listen") {
                this->serverBlock.listen = value;
                return ;
            } else if (directive == "server_name") {
                this->serverBlock.server_name = value;
                return ;
            } else if (directive == "include") {
                this->serverBlock.include = value;
                return ;
            } else if (directive == "location") {
                this->serverBlock.location = value;
                return ;
            }
        }
    }
};

#endif