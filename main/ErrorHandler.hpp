#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include <exception>
#include <ctime>
#include <string>
#include <sys/errno.h>

class ErrorHandler : public std::exception {
	public:
		typedef enum e_ErrCode {NORMAL, ALERT, CRITICAL} ErrCode;
	private:
        std::string _errmsg;
        std::string _at;
		ErrorHandler::ErrCode _errcode;
	public:
		ErrorHandler();
		ErrorHandler(std::string errmsg, ErrorHandler::ErrCode errcode);
		ErrorHandler(std::string errmsg, ErrorHandler::ErrCode errcode, std::string at);
		virtual ~ErrorHandler() throw();
		ErrorHandler::ErrCode getErrorcode(void) const;
		static char* getTime(void);
		virtual const char* what() const throw();
};

#endif
