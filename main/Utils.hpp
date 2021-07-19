#ifndef UTILS_HPP
#define UTILS_HPP

//#include "GlobalConfig.hpp"
#include <string>
#include <map>
#include <sstream>

class Utils {
    public:
        static std::string getMapValue(std::map<std::string, std::string> map_, const std::string& key);
        static std::string randomStringGenerator(int length);
        static int hextoint(std::string str);
        static std::string ltos(long number);
};
#endif