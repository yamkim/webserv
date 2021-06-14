#include "Server.hpp"

Server::Server() : _socket(-1), _port(0), _backlog(0), _enable(false) {
	std::memset(&_address, 0, sizeof(_address));
}

Server::Server(int port, int backlog) : _socket(-1), _port(port), _backlog(backlog), _enable(false) {
	std::memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = htonl(INADDR_ANY);
	_address.sin_port = htons(_port);
}

Server::Server(const char *ip, int port, int backlog) : _socket(-1), _port(port), _backlog(backlog), _enable(false) {
	std::memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = inet_addr(ip);
	_address.sin_port = htons(_port);
}

Server::Server(const Server & server) {
	if (this != &server)
		*this = server;
}

Server & Server::operator=(const Server & server) {
	(void) server;
	return (*this);
}

Server::~Server()
{
}

void Server::makeServer(void) {
	_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (_socket == -1) {
		return ;
	}
	if (bind(_socket, (struct sockaddr *) &_address, sizeof(_address)) == -1) {
		return ;
	}
	if (listen(_socket, _backlog) == -1) {
		return ;
	}
	_enable = true;
}

bool Server::isEnable(void) {
	return (_enable);
}

int & Server::getSocket(void) {
	return (_socket);
}
