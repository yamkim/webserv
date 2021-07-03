#ifndef UTILS_HPP
#define UTILS_HPP

#include "NginxConfig.hpp"

class Utils {
    public:
        static std::string getMapValue(std::map<std::string, std::string> map_, const std::string& key) {
            return map_.find(key)->second;
        }
};
#endif