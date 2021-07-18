#include "Utils.hpp"

std::string Utils::getMapValue(std::map<std::string, std::string> map_, const std::string& key) {
    return map_.find(key)->second;
}

std::string Utils::randomStringGenerator(int length) {
    std::string rtn;
    static bool called = false;
    if (called == false) {
        called = true;
        srand(time(NULL));
    }
    for (int i = 0; i < length; i++) {
        rtn.push_back((std::rand() % ('Z' - 'A') + 'A'));
    }
    return (rtn);
}

int Utils::hextoint(std::string str) {
    int digit = 16;
    int sum = 0;
    int p = 1;
    int pointer = 0;

    while (str[pointer] == '+' || str[pointer] == '-') {
        p = (str[++pointer] == '-') ? (p * -1) : p;
    }
    while ( (str[pointer] >= '0' && str[pointer] <= '9')
            || (str[pointer] >= 'a' && str[pointer] <= 'f')
            || (str[pointer] >= 'A' && str[pointer] <= 'F')) {
        int n;
        if (str[pointer] >= '0' && str[pointer] <= '9') {
            n = str[pointer] - '0';
        } else if (str[pointer] >= 'a' && str[pointer] <= 'f') {
            n = str[pointer] - 'a' + 10;
        } else {
            n = str[pointer] - 'A' + 10;
        }
        sum = n + sum * digit;
        ++pointer;
    }
    return (sum * p);
}

std::string Utils::ltos(long number) {
    std::stringstream ss;
    ss << number;
    return (ss.str());
}
