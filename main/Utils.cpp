#include "Utils.hpp"

std::string Utils::getMapValue(std::map<std::string, std::string> map_, const std::string& key) {
    return map_.find(key)->second;
}