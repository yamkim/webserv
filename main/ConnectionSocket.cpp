#include "ConnectionSocket.hpp"

ConnectionSocket::ConnectionSocket(int listeningSocketFd, const NginxConfig::ServerBlock& serverConf, const NginxConfig::GlobalConfig& nginxConf) : Socket(-1, serverConf), _nginxConf(nginxConf) {
    this->_socket = accept(listeningSocketFd, (struct sockaddr *) &this->_socketAddr, &this->_socketLen);
    if (this->_socket == -1) {
        throw ErrorHandler("Error: connection socket error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    if (getsockname(this->_socket, (struct sockaddr *) &myAddr, &this->_socketLen) == -1) {
        throw ErrorHandler("Error: getsockname error.", ErrorHandler::ALERT, "ConnectionSocket::ConnectionSocket");
    }
    _req = new HTTPRequestHandler(_socket, _serverConf, _nginxConf);
    _res = NULL;
    _dynamicBufferSize = 0;
    _connectionCloseByServer = false;
    _data = new HTTPData();
    setConnectionData(_socketAddr, myAddr);
}

ConnectionSocket::~ConnectionSocket(){
    delete _req;
    delete _res;
    delete _data;
}

HTTPRequestHandler::Phase ConnectionSocket::HTTPRequestProcess(void) {
    HTTPRequestHandler::Phase phase;
    try {
        phase = _req->process(*_data, getDynamicBufferSize());
    } catch (const std::exception& error) {
        /** NOTE
         * 파싱 에러 (데이터는 받았지만 클라이언트가 이상한 데이터를 줄 때) : 400 (error level : NORMAL)
         * 내부 에러 (내부적인 변수 할당 실패 등) : 500 (error level : ALERT / CRITICAL)
         */
        ErrorHandler *err = dynamic_cast<ErrorHandler *>(const_cast<std::exception*>(&error));
        std::cerr << error.what() << std::endl;
        phase = HTTPRequestHandler::FINISH;
        if (err != NULL && err->getErrorcode() == ErrorHandler::NORMAL) {
            _data->_statusCode = 400;
        } else {
            _data->_statusCode = 500;
        }
    }
    if (phase == HTTPRequestHandler::FINISH) {
        if (_data->_HTTPCGIENV.find("HTTP_CONNECTION") != _data->_HTTPCGIENV.end()) {
            if (_data->_HTTPCGIENV["HTTP_CONNECTION"] == std::string("close")) {
                _connectionCloseByServer = true;
            }
        }
        _req->closeFilefd();
        _res = new HTTPResponseHandler(_socket, _serverConf, _nginxConf);
    }
    return (phase);
}

void ConnectionSocket::setConnectionData(struct sockaddr_in _serverSocketAddr, struct sockaddr_in _clientSocketAddr) {
    std::stringstream clientPortString;
    std::stringstream hostPortString;
    _data->_clientIP = std::string(inet_ntoa(_serverSocketAddr.sin_addr));
    clientPortString << ntohs(_serverSocketAddr.sin_port);
    _data->_clientPort = std::string(clientPortString.str());
    _data->_hostIP = std::string(inet_ntoa(_clientSocketAddr.sin_addr));
    hostPortString << ntohs(_clientSocketAddr.sin_port);
    _data->_hostPort = std::string(hostPortString.str());
}

HTTPResponseHandler::Phase ConnectionSocket::HTTPResponseProcess(void) {
    HTTPResponseHandler::Phase phase;
    phase = _res->process(*_data, getDynamicBufferSize());
    if (phase == HTTPResponseHandler::CGI_RUN) {
        // NOTE: CGI일 때 길이가 리턴되면 연결을 끊으면 안될거 같은데...
        _connectionCloseByServer = true;
    } else if (phase == HTTPResponseHandler::FINISH) {
        if (_connectionCloseByServer == false) {
            delete _req;
            delete _res;
            delete _data;
            _req = new HTTPRequestHandler(_socket, _serverConf, _nginxConf);
            _res = NULL;
            _data = new HTTPData();
            setConnectionData(_socketAddr, myAddr);
            phase = HTTPResponseHandler::FINISH_RE;
        }
    }
    return (phase);
}

int ConnectionSocket::runSocket() {
    return (0);
}

int ConnectionSocket::getCGIfd(void) {
    return (_res->getCGIfd());
}

int ConnectionSocket::getFilefd(void) {
    return (_req->getFilefd());
}

void ConnectionSocket::ConnectionSocketKiller(void* connectionsocket) {
    delete reinterpret_cast<ConnectionSocket*>(connectionsocket);
}

long ConnectionSocket::getDynamicBufferSize(void) {
    return (_dynamicBufferSize);
}

void ConnectionSocket::setDynamicBufferSize(long dynamicBufferSize) {
    _dynamicBufferSize = dynamicBufferSize;
}
