#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include <string>
#include <map>
#include <sys/socket.h>
#include "ErrorHandler.hpp"

#define REQUEST_BUFFER_SIZE 100
#define RESPONSE_BUFFER_SIZE 100

class HTTPHandler {
	private:
		HTTPHandler();
    public:
        typedef enum e_Method {UNDEF, GET, POST, DELETE} Method;
	protected:
        static const ssize_t TRANS_ERROR = -1;
        Method _method;
        std::string _URI;
        std::map<std::string, std::string> _headers;
        int _connectionFd;
        std::string _headerString;
	public:
		HTTPHandler(int connectionFd);
		virtual ~HTTPHandler();
};
#endif
