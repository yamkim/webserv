#include "CGISession.hpp"

CGISession::CGISession(const std::string& absolutePath) : _pid(-2), _inputStream(-1), _outputStream(-1) {
    // NOTE: python에 대한 설정이지만, 고정적인 부분이면 생성자 생성시 확장자를 받아오고 분기하는 것이 좋을 거 같습니다!
    std::string _binary;
    std::string _cgiArg;
    std::map<std::string, std::string> _envMap;

    _envMap[std::string("USER")] = std::string(std::getenv("USER"));
    _envMap[std::string("PATH")] = std::string(std::getenv("PATH"));
    _envMap[std::string("LANG")] = std::string(std::getenv("LANG"));
    _envMap[std::string("CONTENT_TYPE")] = std::string("application/x-www-form-urlencoded");
    _envMap[std::string("GATEWAY_INTERFACE")] = std::string("CGI/1.1");
    _binary = std::string("/usr/bin/python3");
    _cgiArg = std::string("data=test");

    // setCGIargs(binary, absolutePath, cgiArg, envMap);
    _arg[0] = const_cast<char*>(_binary.c_str());
	_arg[1] = const_cast<char*>(absolutePath.c_str());
    if (_cgiArg.empty()) {
        _arg[2] = NULL;
    } else {
        _arg[2] = const_cast<char*>(_cgiArg.c_str());
    }
	_arg[3] = NULL;
	_env = generateEnvp(_envMap);
}

CGISession::CGISession(const CGISession & cgisession) {
	if (this != &cgisession)
		*this = cgisession;
}

CGISession & CGISession::operator=(const CGISession & cgisession) {
	(void) cgisession;
	return (*this);
}

CGISession::~CGISession() {
	if (_pid > 0 && kill(_pid, SIGKILL) == -1) {
		throw ErrorHandler("Can't close File Descriptor", ErrorHandler::ALERT, "~CGISession @ _pid");
	}
	if (_inputStream > 0 && close(_inputStream) == -1) {
		throw ErrorHandler("Can't close File Descriptor", ErrorHandler::ALERT, "~CGISession @ _inputStream");
	}
	if (_outputStream > 0 && close(_outputStream) == -1) {
		throw ErrorHandler("Can't close File Descriptor", ErrorHandler::ALERT, "~CGISession @ _outputStream");
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

#if 0
void CGISession::setCGIargs(std::string& binary, const std::string& filename, std::string& cgiarg, std::map<std::string, std::string> env) {
	_arg[0] = const_cast<char*>(binary.c_str());
	_arg[1] = const_cast<char*>(filename.c_str());
    if (cgiarg.empty()) {
        _arg[2] = NULL;
    } else {
        _arg[2] = const_cast<char*>(cgiarg.c_str());
    }
	_arg[3] = NULL;
	_env = generateEnvp(env);
}
#endif

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
