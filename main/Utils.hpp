#ifndef UTILS_HPP
#define UTILS_HPP

//#include "NginxConfig.hpp"
#include <string>
#include <map>

class Utils {
    public:
        static std::string getMapValue(std::map<std::string, std::string> map_, const std::string& key);
        static std::string randomStringGenerator(int length);
        static int hextoint(std::string str);
};
#endif