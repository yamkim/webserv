# rfc3875 표준 요약

## **소개**

CGI는 서버와 CGI 스크립트와 서버가 클라이언트의 요청에 응답할 수 있게 해준다.

클라이언트의 요청은 URI, 요청 메소드(GET, POST, ...), 전송 프로토콜에 의해 제공되는 요청에 대한 부수적인 정보 등으로 구성된다.

CGI는 클라이언트의 요청의 추상적인 매개 변수를 정의하며 CGI 스크립트와 서버 간 플랫폼 독립적인 인터페이스도 정의한다.

서버는 클라이언트의 요청에 대한 연결, 데이터 전송, 네트워크 관련 등을 관리하며 CGI 스크립트는 서버에서 실행되는 프로그램에 대해서만 관리한다.

이 규격은 HTTP/1.1에 정의된 용어를 동일하게 사용하지만 "meta-variable", "script", "server" 이 세가지 용어는 HTTP/1.1에서 정의된 용어와 다르다.

- meta-variable
    - 웹 서버에서 CGI 스크립트로 전달하는 이름이 있는 매개 변수이다. 보통 운영체제의 환경변수로 구현된다. (꼭 그럴 필요는 없다.)
- script
    - 서버가 이 인터페이스에 대해 호출하는 소프트웨어를 말한다. (아마 CGI 스크립트를 말하는듯 하며) CGI에서 스크립트는 보다 넓은 의미를 가진다.
- server
    - 클라이언트의 요청을 서비스하기 위해 CGI 스크립트를 호출하는 웹 서버를 의미한다.

## **표기 규칙 및 일반 문법**

### **Augmented BNF**

- RFC 문서에서 통신 프로토콜을 정의할 때 자주 사용하는 메타 언어이다.

### **기본 룰**

- 문서에서 didit이 0~9를 의미한다던지 NL이 '\n'을 의미한다던지 같은 사항들이 정의되어 있음

### **URL Encoding**

- URL로 데이터를 전송할 때의 인코딩 방법을 규정한다. (RFC 2396에 정의됨)
- 일반적인 URL 인코딩 방법과 같다.

## **(CGI) 스크립트 호출**

### **(웹) 서버의 역할**

서버는 일종의 게이트웨이 역할을 한다.

클라이언트로부터 요청을 받음 → 클라이언트의 요청을 CGI 요청으로 변환 → 스크립트 실행 → 실행에 대한 응답을 클라이언트에 대한 응답으로 변환

으로 흘러간다.

클라이언트의 요청을 처리할 때 적절한 보안 및 인증 구현을 담당할 수 있으며 서버가 중간에 개입해 요청이나 응답을 임의로 수정할 수도 있다.

서버는 CGI 스크립트와 클라이언트 간 적절하게 프로토콜이나 데이터를 잘 변환해야 한다.

### **(CGI) 스크립트 선택**

서버는 클라이언트로부터 받은 URI로 실행할 스크립트를 정한다. URI엔 슬래시로 구분된 계층 경로가 포함되어 있으며 특정 요청에 대해선 개별적인 스크립트를 지정할 수도 있다.

### **Script-URI**

클라이언트의 요청 URI에서 스크립트를 선택하는 것은 서버 각각의 구현 혹은 구성에서 정의된다. (특정 확장자를 가진 파일만 CGI 스크립트로서 동작하게 한다던지)

script-URI는 다음과 같이 정의될 수 있다.

- <스키마> "://" <서버> ":" <포트> <경로> "?" <쿼리 문자열>

### **실행**

스크립트는 시스템이 정의하는 방식으로 열리며 (*.php 파일을 클라이언트에서 요청하면 php 파일이 php-cgi를 통해서 열리는 것과 같은 식으로) 지정하지 않으면 스크립트가 실행파일로서 열린다.

서버는 CGI 요청을 준비하며 (자세한 준비과정은 후술) 이 요청은 요청에 대한 메타변수와 메시지 데이터로 구성된다.

서버는 클라이언트로부터 이 데이터들을 다 받기 전에 스크립트를 실행할 수 있고 스크립트에 대한 응답은 적절하게 (자세한 과정은 후술) 서버로 반환된다.

## **CGI 요청**

요청에 대한 정보는 요청에 대한 메타변수(meta-variable)와 메시지 본문 데이터에서 가져온다.

### **메타변수 요청**

메타 변수는 서버에서 스크립트로 전달된 요청에 대한 데이터를 포함하며 시스템이 정의하는 방식으로 스크립트가 엑세스한다. 메타변수의 이름은 대소문자를 구분하지 않으므로 대소문자만 다른 이름이 있으면 안된다. 메타 변수의 표준 명명법은 다음과 같다. (시스템마다 표현법이 다를수도 있음)

meta-variable-name = "AUTH_TYPE" | "CONTENT_LENGTH" | "CONTENT_TYPE" | "GATEWAY_INTERFACE" | "PATH_INFO" | "PATH_TRANSLATED" | "QUERY_STRING" | "REMOTE_ADDR" | "REMOTE_HOST" | "REMOTE_IDENT" | "REMOTE_USER" | "REQUEST_METHOD" | "SCRIPT_NAME" | "SERVER_NAME" | "SERVER_PORT" | "SERVER_PROTOCOL" | "SERVER_SOFTWARE" | scheme | protocol-var-name | extension-var-name protocol-var-name = ( protocol | scheme ) "_" var-name scheme = alpha*(alpha|digit|"+"|"-"|".") var-name = token extension-var-name = token

위의 메타변수가 권장되며 추가 구현을 하기 위해 확장 메타변수를 사용할 때에는 접두사 "X_"를 붙인다.

- AUTH_TYPE : 사용자를 식별하는 식별자가 들어간다. 클라이언트 프로토콜이나 서버 구현헤 의해 정의된 식별자가 들어가며 대소문자를 가리지 않는다.
- CONTENT_LENGTH : 메시지 본문 데이터의 길이를 의미한다. 본문 데이터가 없으면 설정되지 않거나 빈 값을 가진다.
- CONTENT_TYPE : 메시지 본문 데이터의 타입을 의미한다.
- GATEWAY_INTERFACE : 서버와 스크립트 간 통신할 때의 인터페이스 버전을 의미한다. 이 문서는 CGI/1.1 표준을 정읠하므로 여기선 "CGI/1.1" 으로 고정한다.
- PATH_INFO : CGI 스크립의 경로가 들어간다.
- PATH_TRANSLATED : URI의 CGI 스크립트의 경로 값을 로컬의 경로 값으로 변환한 값이 들어간다. PATH_INFO가 NULL이면 이 변수도 NULL이다.
- QUERY_STRING : 쿼리의 변수가 들어간다.
- REMOTE_ADDR : 요청한 클라이언트의 주소가 들어간다.
- REMOTE_HOST : 클라이언트의 정규화된 도메인 이름이 들어간다.
- REMOTE_IDENT : 클라이언트의 ID값이 들어간다. (없어도 됨)
- REMOTE_USER : 사용자를 식별하기 위한 ID가 들어간다.
- REQUEST_METHOD : HTTP 메소드 값 (GET , POST, HEAD, ...) 이 들어간다.
- SCRIPT_NAME : CGI 스크립트의 경로를 의미한다.
- SERVER_NAME : 서버 호스트의 이름(IP)이 들어간다.
- SERVER_PORT : 서버의 포트번호가 들어간다.
- SERVER_PROTOCOL : 서버의 프로토콜이 들어간다. HTTP 1.1이면 "HTTP/1.1" 이 들어간다.
- SERVER_SOFTWARE : 서버 프로그램의 이름과 버전이 들어간다.

### **메시지 본문 데이터 (Request Message-Body)**

요청 데이터 (본문 데이터)는 스크립트가 시스템 정의 방식으로 접근하며 별도로 정의하지 않으면 "표준 입력"으로 읽어들인다.

### **요청 메소드**

메타 변수를 스크립트에 제공할 때 적용할 처리 방법을 식별한다.

- GET : GET 요청에 대해 문서를 반환하는 이상의 조치를 하면 안된다. (GET 메소드는 "idempotent" 하기 때문)
- POST : 메타 변수 외에 스크립트에 대한 특정한 처리를 요청할 수 있고 POST가 전송하는 메시지를 기반으로 문서를 반환한다. 서버에 일반적으로 영구적인 영향을 미친다. 반드시 CONTENT_LENGTH를 체크해야 한다.

### **스크립트 명령줄**

일부 시스템은 CGI 스크립트에 명령줄을 전달할 수 있다. HTTP 쿼리의 값에 대해 식별되는 값이 명령줄로 들어간다. (위 Script-URI의 쿼리 문자열이 명령줄로 들어간다.) "=" 문자는 들어가면 안된다.

## **NPH Script**

NPH (Non-Parsed Header) 스크립트는 응답 헤더가 스크립트 내에서 생성되는 스크립트를 의미한다.

### **식별**

CGI 표준은 출력 데이터로 NPH인지 구별하는 메커니즘을 제공하지 않으며 이런 것들은 서버가 알아서 구현해야 한다.

### **NPH 응답**

스크립트의 응답을 서버로 보내려면 시스템이 정의한 방식이 있어야 하며 정의되어있지 않으면 CGI와 동일하다.

## **CGI 응답**

CGI 스크립트는 응답 타입에 맞게 헤더를 붙여야 한다.

### **응답 핸들링**

일반적으로 "표준 출력"을 통해 CGI 스크립트의 응답을 받는다. 요청을 처리할 때 스크립트는 "REQUEST_METHOD" 변수를 참조해야 하며 서버는 데이터를 수신해야 하는데 시간 제한을 둘수도 있다.

### **응답 타입**

응답은 메시지 헤더와 본문으로 구분되며 개행으로 구분된다. 스크립트는 문서에 대한 응답이던 리다이렉션 응답이던 응답이 있어야 한다.

- 문서에 대한 응답
    - CGI 스크립트는 응답에 대한 성공 여부와 함께 응답을 보낸다.
    - Content-Type 헤더 필드를 반드시 삽입해야 한다.
        - 양식은 document-response = Content-Type [ Status ] *other-field NL response-body 와 같다.
        - 예를 들면 리턴 값이 html일 경우 "Content-type: text/html\n"을 붙이고 본문을 반환한다.
- 로컬 리다이렉트 응답
- 클라이언트 리다이렉트 응답
- 문서가 추가된 클라이언트 리다이렉트 응답

### **응답 헤더**

위에서 언급한 Content-Type 같은건 응답 헤더에 속한다.헤더는 다음과 같은 것들이 올 수 있다.

- Content-Type (사실상 필수)
- Location
- Status
- Protocol-Specific Header Fields
- 확장 헤더 필드 (이 경우는 접두사 "X-CGI-"를 붙인다)

### **응답 메시지 본문**

응답 메시시 본문은 클라이언트로 송부될 문서이다. 서버는 CGI 스크립트가 EOF에 도달할때까지 스크립트의 데이터를 읽고 메시지 본문은 HEAD 요청, 요구된 전송 코드, 문자 집합 변환을 제외하고는 임의로 변조하면 안된다.

## **시스템 명세**

### **UNIX**

- 메타변수
    - 메타변수는 환경변수로 전달되며 C 표준 라이브러리의 getenv() 함수를 이용하거나 메인문 인자인 environ 변수로 접근한다.
- 명령줄
    - main문 인자의 argc, argv로 접근한다.
- 현재 작업중인 디렉터리
    - 스크립트를 포함하는 디렉터리로 설정한다.
- 문자열 셋
    - 아스키 코드를 기반으로 하며 PATH_TRANSLATED엔 NULL을 제외한 값이 들어갈 수 있다. 개행은 "\r\n"이다.

### **POSIX**

- 유닉스와 유사하므로 생략

## **예제 코드 (shell)**

cgi를 쉘에서 동작시킨 예제이다. 메타변수는 환경변수로 입력하고 url의 쿼리문은 매개변수로, 요청 데이터는 표준 입력으로 입력한다. php-cgi를 이용하여 실행시킬 수 있다.

```
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
export SCRIPT_NAME="idx.php"
#export SERVER_NAME=
#export SERVER_PORT=
export SERVER_SOFTWARE=WEBSERV/0.1
export SERVER_PROTOCOL=HTTP/1.1
export DOCUMENT_ROOT="/Users/joohongpark/Desktop/php-mac/bin/"
export DOCUMENT_URI="/Users/joohongpark/Desktop/php-mac/bin/phpliteadmin.php"
export REQUEST_URI="/Users/joohongpark/Desktop/php-mac/bin/phpliteadmin.php"

# it's not in CGI/1.1 standard
export SCRIPT_FILENAME="/Users/joohongpark/Desktop/php-mac/bin/phpliteadmin.php"
export REDIRECT_STATUS=200

#echo "pval=1234&name=joopark" | ./php-cgi "val=value"
```