#ifndef CGISESSION_H
#define CGISESSION_H

#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <unistd.h>
#include <signal.h>
#include "ErrorHandler.hpp"
#include "HTTPData.hpp"

// CGI Session (draft) by joopark

class CGISession {
	private:
		pid_t _pid;
		int _inputStream;
		int _outputStream;
		char **_env;
		char *_arg[4];
        char** generateEnvp(std::map<std::string, std::string>& arg);
		CGISession();

	public:
		CGISession(HTTPData& data);
		// CGISession(const CGISession & cgisession);
		~CGISession();
		CGISession & operator=(const CGISession & cgisession);
		int & getInputStream(void);
		int & getOutputStream(void);
		// void setCGIargs(std::string& binary, const std::string& filename, std::string& cgiarg, std::map<std::string, std::string> env);
		void makeCGIProcess();
};

#endif
