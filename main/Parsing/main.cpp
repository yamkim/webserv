#include <iostream>
#include "NginxConfig.hpp"

#define CONF_PATH "./conf/nginx.conf"

int main(int argc, char *argv[])
{
    const char* confPath = static_cast<const char *>(CONF_PATH);
    if (argc == 2) {
        confPath = argv[1];
    } else if (argc > 2) {
        std::cerr << "\033[0;31m[err] bad grgument!\033[0m" << std::endl;
        return (1);
    }
    // NOTE: nginx.conf 파싱 GUIDE LINE
    // - 주석인 경우 처리하는 방식
    // - ';'을 사용하지 않았을 때
    // - 없는 값에 접근할 때
    // - 숫자인 경우
    // - 매개변수의 개수
    // - 뒤에서, 사용하지 않는 변수들을 어떻게 대체할 것인지
    try {
        NginxConfig::NginxConfig nginxConfig(confPath);
        std::cout << "[DEBUG] types: " << nginxConfig._http.types.typeMap["html"] << std::endl;
        std::cout << "[DEBUG] charset: " << nginxConfig._http.dirMap["charset"] << std::endl;
        std::cout << "[DEBUG] server[index]: " << nginxConfig._http.server[0].index[0] << std::endl;
        std::cout << "[DEBUG] server[index]: " << nginxConfig._http.server[0].index[1] << std::endl;
        std::cout << "[DEBUG] server[error_page]: " << nginxConfig._http.server[0].error_page[0] << std::endl;
        // std::cout << "[DEBUG] nginxConfig location[index]: " << nginxConfig._http.server[0].location[0].index[0] << std::endl;
    } catch (const std::string& error) {
        std::cout << error << std::endl;
        return (1);
    }
}