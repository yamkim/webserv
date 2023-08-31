# C++98 Web Server

## 개요

본 프로젝트는 **비동기 논블러킹 (Asynchronous Non-blocking I/O)** 웹 서버를 만드는 프로젝트입니다.

C++ 98 문법을 기반으로 Mac OS에서 동작되며 nginx의 동작과 유사하며 HTTP 1.1을 지원합니다.

C++98 Web Server는 **단일 프로세스 및 단일 스레드**만 사용합니다.

## 기능

- HTTP 1.1 표준 기반의 웹서버
- 정적 파일에 대한 HTTP 서버 기능
- CGI 표준 지원 (PHP-CGI, Perl 등 사용 가능)
    - FastCGI는 지원하지 않음
- 존재하지 않는 리소스에 대한 에러 표시 지원
- 리다이렉트 지원
- 특정 리소스에 대한 AutoIndex 지원
- `Transfer-Encoding: chunked` 지원
- nginx와 유사한 설정 파일 문법 지원
- 클라이언트가 응답이 없을시 연결 종료 (Timeout)

## 실행방법
```shell
$ git clone git@github.com:yamkim/webserv.git
$ chmod +x run.sh && ./run.sh
```
- `conf/` 폴더에는 설정을 위한 파일들이 있습니다. 
- `./run.sh` 쉘파일 실행 시, `conf/default.conf.template` 파일을 기준으로 웹서브를 설정하기 때문에, 필요시 해당 설정파일을 수정합니다.

## Docs
go to [링크](./docs/README.md)