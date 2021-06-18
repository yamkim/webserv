#include <iostream>
#include "Parser.hpp"

int main(void) {
    std::string buf;
    try {
        Parser parser("requestEx.txt");
        parser.showRawData();
        parser.setConfigurationMap();
        parser.showConfigurationMap();
    //    buf = getFileData("requestEx.txt");
    } catch (const char * error) {
        std::cout << error << std::endl;
    }


    return (0);
}
