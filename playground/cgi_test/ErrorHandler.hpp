#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

// NOTE 에러 양식 : *시간 [*에러종류] : *에러 종류(*errno에 대한 에러 메시지) (in *발생 위치)

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
		ErrorHandler(const ErrorHandler & errorhandler);
		virtual ~ErrorHandler() throw();
		ErrorHandler & operator=(const ErrorHandler & errorhandler);
		ErrorHandler::ErrCode getErrorcode(void) const;
		static char *getTime(void);
		virtual const char* what() const throw();
};

#endif
