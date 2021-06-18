#include <iostream>
#include "HttpParser.hpp"

int main(void) {
    try {
        HttpParser parser("requestEx.txt");
        parser.showRawData();
        parser.setConfigurationMap();
        parser.showConfigurationMap();
    } catch (const char * error) {
        std::cout << error << std::endl;
    }

    return (0);
}
