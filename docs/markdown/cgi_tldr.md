# CGI (RFC3875) 개요

## CGI란?

CGI의 뜻은 "공통 게이트웨이 인터페이스" 라는 뜻이다. 그러니까 일종의 인터페이스라는 말이며 클라이언트의 요구에 따라 서버 내에 특정 프로그램을 실행시키는 역할을 하게 할 수 있는 인터페이스를 의미한다.

보통의 웹 서버는 서버 내부에 있는 정적 파일을 클라이언트에 보내는 역할을 하는데 CGI는 서버 내의 프로그램에 대한 요청 클라이언트에게 받으면 프로그램의 응답을 클라이언트에게 보내는 것이 다르다.

CGI 표준은 rfc3875에 명시되어 있다.

## 이해를 위한 사전 요약

- 기존의 파일 전송만을 하는 웹 서버가 동적인 역할을 수행할 필요성이 생김
    - 예를 들면 서버의 데이터를 조회한 결과를 리턴하거나 그에 따른 동적인 HTML 생성이 필요
- 그래서 웹 서버 프로그램과 다른 프로그램 (PHP, Perl 등)과 통신하여 동적인 페이지 생성은 다른 프로그램에 위탁함
- 클라이언트에게 요청이 날라오면 서버 프로그램이 CGI 프로토콜 기반으로 다른 프로그램에게 요청을 날리고 요청에 대한 응답을 클라이언트에게 그대로 전송함.
- 서버 프로그램은 CGI 프로그램 (PHP, Perl 등)의 프로세스를 실행시키고 해당 프로세스에게 표준 입력, 환경 변수, 실행 인자로 어떤 동작을 할지 알려준다.
- 실행된 CGI 프로그램은 표준 출력을 통해 결과물을 출력하며 웹 서버 프로그램은 결과물을 클라이언트에게 전달한다.

## CGI의 동작 순서

### 1. 클라이언트의 요청

클라이언트는 웹 서버에 url을 통해 요청을 날린다.

예를 들면 아래 예제와 같이 GET이나 POST로 서버에 요청을 보낼 수 있다.

```cpp
GET /index.php?data=value HTTP/1.1
Host: foo.com
Content-Type: application/x-www-form-urlencoded
```

```cpp
POST /test.php HTTP/1.1
Host: foo.com
Content-Type: application/x-www-form-urlencoded
Content-Length: 13

pval=1234&name=joopark
```

### 2. 서버의 요청 파싱

위 요청을 받으면 웹 서버는 정해진 규칙에 의해 이 요청이 CGI인지 확인해야 한다. Nginx에선 확장자가 .php인 요청을 별도로 처리하게 할 수 있다.

최근의 nginx는 기존 CGI 방식을 사용하지 않으며 FastCGI라는 방식을 사용하므로 주의해야 한다.

서버가 리눅스 혹은 유닉스 기반 OS에서 동작하고 해당 요청이 CGI이면 HTTP 헤더를 파싱해서 다음과 같이 분류한다. 만약 위와 같이 요청이 날라온다면 다음과 같이 분류할 수 있다.

GET 헤더에 대한 분류는 다음과 같다.

- GET
    - Meta-Variables (환경 변수로 전달되는 변수들)
        - GATEWAY_INTERFACE : CGI/1.1
        - PATH_INFO : [php 스크립트를 실행할 php-cgi의 절대경로]
        - QUERY_STRING : data=value
        - REQUEST_METHOD : GET
        - SCRIPT_NAME : test.php
        - ...
    - command line (CGI 프로그램의 인자로 전달되는 데이터)
        - data=value

POST 헤더에 대한 분류는 다음과 같다.

- POST
    - Meta-Variables (환경 변수로 전달되는 변수들)
        - CONTENT_LENGTH : 22
        - GATEWAY_INTERFACE : CGI/1.1
        - PATH_INFO : [php 스크립트를 실행할 php-cgi의 절대경로]
        - QUERY_STRING : data=value
        - REQUEST_METHOD : POST
        - SCRIPT_NAME : test.php
        - ...
    - command line (CGI 프로그램의 인자로 전달되는 데이터)
        - (NULL)
    - Request Message-Body (CGI 프로그램의 표준 입력으로 전달되는 데이터)
        - pval=1234&name=joopark

항목을 보면 Meta-Variables와 command line과 Request Message-Body가 있다.

- Meta-Variables : php 스크립트를 실행할 프로세스(php-cgi)에 대해 설정되어야 할 환경변수 값
- command line : php-cgi의 환경변수 인자로 들어가야 하는 값
- Request Message-Body : php-cgi의 표준 입력으로 들어가는 값

### 3. 서버의 요청 실행

만약 CGI가 아니라면 서버는 단순히 서버 내의 정적 파일(웹페이지 등)을 반환하거나 하는 동작을 할 것이다. 하지만 CGI는 서버 내에서 특정 프로세스를 실행한 값을 반환해야 하므로 먼저 프로세스를 실행시킨다.

실제로는 시스템 콜 등을 이용해서 실행하겠지만 비슷한 동작을 쉘 스크립트로 짜보면 다음과 같이 동작할 것이다.

```bash
#!/bin/bash

# CGI/1.1 Standard meta-variable
#export AUTH_TYPE=
export CONTENT_LENGTH=22
export CONTENT_TYPE="application/x-www-form-urlencoded"
export GATEWAY_INTERFACE="CGI/1.1"
#export PATH_TRANSLATED=
export PATH_INFO="/Users/joohongpark/Desktop/php-mac/bin"
export QUERY_STRING="val=1234"
#export REMOTE_ADDR=127.0.0.1
#export REMOTE_HOST=127.0.0.1
#export REMOTE_IDENT=
#export REMOTE_USER=
export REQUEST_METHOD=POST
export SCRIPT_NAME="test.php"
#export SERVER_NAME=
#export SERVER_PORT=
export SERVER_SOFTWARE=WEBSERV/0.1
export SERVER_PROTOCOL=HTTP/1.1

# it's not CGI/1.1 standard
export SCRIPT_FILENAME="/Users/joohongpark/Desktop/php-mac/bin/idx.php"
export REDIRECT_STATUS=200

echo "pval=1234&name=joopark" | ./php-cgi "./test.php"  "val=value"
```

스크립트를 보면 알겠지만 Meta-Variables을 설정하기 위해 환경변수를 셋팅하고 command line은 프로세스의 입력 인수로 삽입하며 Request Message-Body는 프로세스의 표준 입력으로 집어 넣는다.

### 4. 서버의 요청 응답

위 프로세스를 실행하고 나면 무엇인가가 표준 출력에 출력이 될 것이다. 웹 서버는 표준 출력을 통해 나오는 내용을 적절한 헤더를 추가하여 그대로 클라이언트에 전달한다.

# Nginx의 CGI

## 기존 CGI의 한계

CGI의 표준에는 요청이 들어올 때 CGI 프로세스를 실행시키는 것으로 명시되어 있다. 프로세스를 실행시키는 동작은 자원 소모가 심하므로 동작이 느리다는 단점이 있다.

만약 같은 서버에 여러개의 CGI 요청이 들어온다면 요청 개수만큼 프로세스를 실행시켜야 하므로 급속도로 느려지게 될 것이다.

## FastCGI

FastCGI는 실행시키고자 하는 CGI 프로그램을 **미리 실행시켜 놓고** IPC (주로 unix socket 또는 TCP socket)를 통해 웹 서버 프로그램과 CGI 프로그램이 통신하는 방식이다. 그래서 RFC의 CGI 표준과 상이하다.

## Nginx의 CGI

Nginx는 기존 방식의 CGI를 지원하지 않으며 FastCGI만을 지원한다. 그래서 모든 CGI는 FastCGI 규격으로 동작된다. 그래서 Nginx에서 기존 CGI (서버의 프로세스를 실행하는 동작) 방식대로 동작시키려면 FCGI Wrapper(fcgiwrap)를 사용해야 하거나 아파치 서버에서 구동하는 등의 다른 방식을 찾아보아야 한다.