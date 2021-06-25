#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stack>
#include "NginxConfig.hpp"


int main() {
    // std::cout << nginx.getRawData() << std::endl;

    try {
        NginxConfig nginx("nginx.conf");
    } catch (const std::string& e) {
        std::cout << e << std::endl;
    }
    
    return (0);
}