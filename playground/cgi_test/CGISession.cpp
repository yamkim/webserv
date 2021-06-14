#include "CGISession.hpp"

// CGI Session (draft) by joopark

CGISession::CGISession() : _pid(-2), _inputStream(-1), _outputStream(-1) {}

CGISession::CGISession(const CGISession & cgisession) {
	if (this != &cgisession)
		*this = cgisession;
}

CGISession & CGISession::operator=(const CGISession & cgisession) {
	(void) cgisession;
	return (*this);
}

CGISession::~CGISession() {}

int & CGISession::getInputStream(void) {
	return (_inputStream);
}

int & CGISession::getOutputStream(void) {
	return (_outputStream);
}

void CGISession::setCGIargs(char *binary, char *filename, char *cgiarg, char **env) {
	_arg[0] = binary;
	_arg[1] = filename;
	_arg[2] = cgiarg;
	_arg[3] = NULL;
	_env = env;
}

void CGISession::makeCGIProcess() {
	int pairForI[2];
	int pairForO[2];

	// 새 프로세스와 통신할 파이프를 만듭니다.
	if (pipe(pairForI) == -1 || pipe(pairForO) == -1) {
		_pid = -1;
		return ;
	}
	_inputStream = pairForI[1];
	_outputStream = pairForO[0];
	if ((_pid = fork()) < 0) {
		return ;
	}
	if (_pid == 0) {
		if ((dup2(pairForI[0], STDIN_FILENO) < 0)
			|| (dup2(pairForO[1], STDOUT_FILENO) < 0)
			|| (close(pairForI[1]) < 0)
			|| (close(pairForO[0]) < 0)) {
			std::exit(1);
		}
		if (execve(_arg[0], _arg, _env) == -1) {
			std::exit(1);
		}
	} else {
		if ((close(pairForI[0]) < 0)
			|| (close(pairForO[1]) < 0)) {
			_pid = -1;
		}
	}
}
