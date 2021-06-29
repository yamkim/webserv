#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include <string>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include "ErrorHandler.hpp"

#define REQUEST_BUFFER_SIZE 100
#define RESPONSE_BUFFER_SIZE 1000

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
        std::string getExtension(void);
        void setGeneralHeader(std::string status);
        void setTypeHeader(std::string type);
        void setLengthHeader(long contentLength);
        void convertHeaderMapToString(bool isCGI);
		virtual ~HTTPHandler();
};
#endif
