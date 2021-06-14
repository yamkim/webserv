#ifndef SERVER_H
#define SERVER_H

#include <cstring>
#include <arpa/inet.h>

// Server generator (draft) by joopark
// TODO: 일부 에러상황 Exception으로 핸들링

class Server {
	private:
		int _socket;
		int _port;
		int _backlog;
		bool _enable;
		struct sockaddr_in _address;
	public:
		Server();
		Server(int port, int backlog);
		Server(const char *ip, int port, int backlog);
		Server(const Server & server);
		~Server();
		Server & operator=(const Server & server);
		void makeServer(void);
		bool isEnable(void);
		int & getSocket(void);
};

#endif
