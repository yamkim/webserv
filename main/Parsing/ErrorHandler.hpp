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
		const char* _errmsg;
		const char* _at;
		ErrorHandler::ErrCode _errcode;
	public:
		ErrorHandler();
		ErrorHandler(const char* errmsg, ErrorHandler::ErrCode errcode);
		ErrorHandler(const char* errmsg, ErrorHandler::ErrCode errcode, const char* at);
		// ErrorHandler(const std::string& errmsg, ErrorHandler::ErrCode errcode, const char* at);
		virtual ~ErrorHandler() throw();
		ErrorHandler::ErrCode getErrorcode(void) const;
		static char* getTime(void);
		virtual const char* what() const throw();
};

#endif
