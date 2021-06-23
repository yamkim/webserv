#include "CGISession.hpp"

// CGI Session (draft) by joopark

CGISession::CGISession() : _pid(-2), _inputStream(-1), _outputStream(-1), _env(NULL) {}

CGISession::CGISession(const CGISession & cgisession) {
	if (this != &cgisession)
		*this = cgisession;
}

CGISession & CGISession::operator=(const CGISession & cgisession) {
	(void) cgisession;
	return (*this);
}

CGISession::~CGISession() {
	if (_pid > 0) {
		if (kill(_pid, SIGKILL) == -1) {
			throw ErrorHandler("Can't close File Descriptor", ErrorHandler::ALERT, "~CGISession @ _pid");
		}
	}
	if (_inputStream > 0) {
		if (close(_inputStream) == -1) {
			throw ErrorHandler("Can't close File Descriptor", ErrorHandler::ALERT, "~CGISession @ _inputStream");
		}
	}
	if (_outputStream > 0) {
		if (close(_outputStream) == -1) {
			throw ErrorHandler("Can't close File Descriptor", ErrorHandler::ALERT, "~CGISession @ _outputStream");
		}
	}
    if (_env != NULL) {
        int i = 0;
        while (_env[i] != NULL) {
            delete _env[i];
            i++;
        }
        delete _env;
    }
}

int & CGISession::getInputStream(void) {
	return (_inputStream);
}

int & CGISession::getOutputStream(void) {
	return (_outputStream);
}

void CGISession::setCGIargs(char *binary, char *filename, char *cgiarg, std::map<std::string, std::string> env) {
	_arg[0] = binary;
	_arg[1] = filename;
	_arg[2] = cgiarg;
	_arg[3] = NULL;
	_env = generateEnvp(env);
}

void CGISession::makeCGIProcess() {
	int pairForI[2];
	int pairForO[2];

	if (pipe(pairForI) == -1 || pipe(pairForO) == -1) {
		throw ErrorHandler("Can't make Pipe", ErrorHandler::ALERT, "CGISession::makeCGIProcess");
	}
	_inputStream = pairForI[1];
	_outputStream = pairForO[0];
	if ((_pid = fork()) < 0) {
		throw ErrorHandler("Can't make Process", ErrorHandler::ALERT, "CGISession::makeCGIProcess");
	}
	if (_pid == 0) {
		if ((dup2(pairForI[0], STDIN_FILENO) == -1) || (dup2(pairForO[1], STDOUT_FILENO) == -1)) {
			throw ErrorHandler("Can't duplicate File Descriptor", ErrorHandler::ALERT, "CGISession::makeCGIProcess");
		}
		if ((close(pairForI[1]) == -1) || (close(pairForO[0]) == -1)) {
			throw ErrorHandler("Can't close File Descriptor", ErrorHandler::ALERT, "CGISession::makeCGIProcess");
		}
		if (execve(_arg[0], _arg, _env) == -1) {
			throw ErrorHandler("Can't duplicate File Descriptor", ErrorHandler::NORMAL, "CGISession::makeCGIProcess");
		}
	} else {
		if ((close(pairForI[0]) == -1) || (close(pairForO[1]) == -1)) {
			throw ErrorHandler("Can't close File Descriptor", ErrorHandler::ALERT, "CGISession::makeCGIProcess");
		}
	}
}

char** CGISession::generateEnvp(std::map<std::string, std::string>& arg) {
    char** rtn = new char*[arg.size() + 1];
    std::map<std::string, std::string>::iterator iter;
    std::string tmpString;
    int i = 0;
    for (iter = arg.begin(); iter != arg.end(); iter++) {
        tmpString = std::string(iter->first);
        tmpString += std::string("=");
        tmpString += std::string(iter->second);
        char* variable = new char[tmpString.size() + 1];
        std::strcpy(variable, tmpString.c_str());
        rtn[i] = variable;
        i++;
    }
    rtn[i] = NULL;
    return (rtn);
}
