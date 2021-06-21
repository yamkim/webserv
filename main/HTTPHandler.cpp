#include "HTTPHandler.hpp"

HTTPHandler::HTTPHandler(int connectionFd) {
	_method = UNDEF;
	_URI = std::string("");
	_connectionFd = connectionFd;
	_headerString = std::string("");
}

HTTPHandler::~HTTPHandler() {}
