#ifndef CGISESSION_H
#define CGISESSION_H

#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include "ErrorHandler.hpp"

// CGI Session (draft) by joopark

class CGISession {
	private:
		pid_t _pid;
		int _inputStream;
		int _outputStream;
		char **_env;
		char *_arg[4];
	public:
		CGISession();
		CGISession(const CGISession & cgisession);
		~CGISession();
		CGISession & operator=(const CGISession & cgisession);
		int & getInputStream(void);
		int & getOutputStream(void);
		void setCGIargs(char *binary, char *filename, char *cgiarg, char **env);
		void makeCGIProcess();
};

#endif
