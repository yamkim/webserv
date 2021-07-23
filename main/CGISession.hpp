#ifndef CGISESSION_H
#define CGISESSION_H

#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <unistd.h>
#include <signal.h>
#include "ErrorHandler.hpp"
#include "HTTPHandler.hpp"
#include "HTTPData.hpp"

#ifndef WEBSERV_VERSION
#define WEBSERV_VERSION "0.0.0"
#endif

class CGISession {
    private:
        pid_t _pid;
        int _inputStream;
        int _outputStream;
        char** _env;
        char* _arg[4];
        char** generateEnvp(std::map<std::string, std::string>& arg);
        CGISession();

    public:
        CGISession(HTTPData& data);
        ~CGISession();
        int& getInputStream(void);
        int& getOutputStream(void);
        void makeCGIProcess(int inputfd);
};

#endif
