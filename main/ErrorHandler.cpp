#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler() : _errmsg(NULL), _at(NULL), _errcode(NORMAL) {}

ErrorHandler::ErrorHandler(const char* errmsg, ErrorHandler::ErrCode errcode) : _errmsg(errmsg), _at(NULL), _errcode(errcode) {}

ErrorHandler::ErrorHandler(const char* errmsg, ErrorHandler::ErrCode errcode, const char* at) : _errmsg(errmsg), _at(at), _errcode(errcode) {}

ErrorHandler::~ErrorHandler() throw() {}

ErrorHandler::ErrCode ErrorHandler::getErrorcode(void) const {
	return (_errcode);
}

char * ErrorHandler::getTime(void) {
	static char timeBuffer[20];
	time_t rawtime;
	struct tm *timeinfo;

	std::time(&rawtime);
	timeinfo = std::localtime(&rawtime);
	std::strftime(timeBuffer, 20, "%Y/%m/%d %H:%M:%S", timeinfo);
	return (timeBuffer);
}

const char* ErrorHandler::what() const throw() {
	static std::string rtn;

    rtn.clear();
	rtn.append(getTime());
	if (_errcode == NORMAL) {
		rtn.append(" [normal] ");
	} else if (_errcode == ALERT) {
		rtn.append(" [alert] ");
	} else if (_errcode == CRITICAL) {
		rtn.append(" [critical] ");
	}
	rtn.append(_errmsg);
	if (errno != 0) {
		rtn.append(" (");
		rtn.append(std::strerror(errno));
		rtn.append(")");
	}
	if (_at != NULL) {
		rtn.append(" at ");
		rtn.append(_at);
	}
	return (rtn.c_str());
}
