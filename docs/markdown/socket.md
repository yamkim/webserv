> 💡 소켓 프로그래밍에 대한 지식과 예제를 다룹니다.

# TCP 통신을 위한 소켓 프로그래밍 연구

## 소켓 프로그래밍이란?

현대 OS는 사실상 거의 다 TCP/IP에 기반한 네트워크 통신을 지원한다. 넓은 의미에서는 운영체제에서 제공하는 API를 호출하여 IP 주소를 기반으로 클라이언트와 서버 간 네트워크 통신을 하도록 해 주는 것을 말한다.

소켓 프로그래밍을 이용하면 접속하고자 하는 서버에 IP를 통해 접속할 수 있으며 본인이 서버가 될 수도 있다.

소켓 프로그래밍을 이용하면 클라이언트 혹은 서버와 TCP 혹은 UDP 패킷을 주고받을 수 있다.

OSI 7 layer에서 "소켓 프로그래밍"을 이용해 응용 계층 ~ 전송 계층(혹은 네트워크 계층)까지 손 댈수 있다.

참고로 패킷(packet)은 네트워크 전송의 용량 단위로, Package와 Bucket의 합성어이다.

# 소켓은?

소켓은 운영체제가 제공하는 일종의 API이다. (윈도우의 winsock도 비슷하지만) 유닉스 계열에서 제공하는 파일 디스크립터를 통해 다른 네트워크에 존재하는 프로그램 간 정보교환을 가능하게 해 주는 방법이다. 그래서 read 혹은 write 시스템 콜을 이용해 데이터를 읽고 쓰게 된다.

소켓은 전송 계층에 관여할 수 있는 API를 제공해 프로그래머가 직접 TCP 패킷을 만들 필요 없이 TCP를 통해 송수신 할 데이터에만 신경을 쓸 수 있다. read와 write(혹은 send와 read를 사용할 수 있음)로 데이터를 읽고 쓰지만 사용자가 직접 TCP나 UDP 패킷 구조를 만들 필요는 없다.

# 소켓 API

## socket (system call)

```c
int socket(int domain, int type, int protocol)
```

소켓 시스템 콜의 역할은 접속하고자 하는 종단점의 파일 디스크립터를 반환해 준다.

인수는 도메인, 타입, 프로토콜이 있다.

- 도메인
    - 도메인 인수는 통신을 하고자 하는 범위를 의미한다. UNIX 계열은 한 시스템 내의 프로세스끼리 통신하기 위한 유닉스 소켓도 있고 IPv4 혹은 IPv6도 있으며 네트워크 장비에 Raw하게 접근할 수 있게 해 주는 도메인도 있다.
    - 인수가 숫자로 되어 있는데 이 숫자는 define 으로 정의되어 있으며 우리는 PF_INET 를 이용한다.
- 타입
    - 보통 세 가지 타입을 지원한다. (운영체제/커널에 따라 그 이상일수도 있다.)
    - SOCK_STREAM
        - 시퀀스가 존재하는 1:1 연결 지향의 연결을 제공한다. 사실상 TCP 전송 방식을 의미한다
    - SOCK_DGRAM
        - 연결 지향성이 아닌 단순한 데이터그램을 전달한다. 사실상 UDP 전송 방식을 의미한다.
    - SOCK_RAW
        - TCP 혹은 UDP가 아닌 직접 패킷을 조작할 때 주로 사용한다.
    
    HTTP는 TCP 기반의 프로토콜이므로 SOCK_STREAM을 사용한다.
    
- 프로토콜
    - 소켓에 사용될 프로토콜을 지정한다. 하지만 도메인이 IP이고 타입이 TCP 혹은 UDP이면 이미 프로토콜이 지정된 것과 마찬가지이므로 이런 경우엔 0으로 set 한다. 다른 도메인 혹은 타입을 기입한다면 그때그때 다르다.

시스템 콜이므로 뭔가 문제가 있으면 -1을 리턴하며 임의의 errno를 set한다.

## connect (system call)

```c
int connect(int socket, const struct sockaddr *address, socklen_t address_len);

struct sockaddr {
	unsigned char sa_len;         /* total length */
	unsigned char sa_family;      /* [XSI] address family */
	char          sa_data[14];    /* [XSI] addr value (actually larger) */
};

struct sockaddr_in {
	__uint8_t       sin_len;
	sa_family_t     sin_family;
	in_port_t       sin_port;
	struct  in_addr sin_addr;
	char            sin_zero[8];
};
```

connect 시스템 콜은 인수로 입력된 소켓 디스크립터를 이용해 address에 접속해 주는 역할을 한다.

소켓의 프로토콜에 따라 조금씩 연결하는 형태가 다르다.

인수는 세가지이며 소켓 파일 디스크립터, 주소 구조체, 주소 구조체의 길이를 받는다.

- 소켓
    - UNIX 시스템답게 파일 디스크립터와 비슷한 형태로 받는다. socket 시스템 콜로 생성된 디스크립터를 삽입한다.
- 주소 구조체
    - 말 그대로 어디에 접속되는지에 대한 정보가 들어있다. **여기서 중요한 포인트가 저 socketaddr의 구조체는 소켓에 따라 변할 수 있다는 것이다. 헤더에 명시된 socketaddr 포인터 타입은 일종의 "다형성" 을 위해 존재하는 것이다.** 만약 IPv4 기반으로 통신한다면 위에 작성된 sockaddr_in 타입으로 주소를 적어야 하며 저 구조체는 socketaddr 포인터 타입으로 형변환되어 인수에 입력되게 된다. (실제로는 아니지만) OOP의 부모 - 자식 간의 다형성으로 이해하면 쉽다.
    - 만약에 IPv6의 주소로 접속한다면 sockaddr_in6 구조체 타입으로 주소를 작성한 후의 그 구조체의 포인터를 socketaddr 포인터로 강제로 형변환 한 다음 인수에 집어넣어야 한다.
    - 만약 소켓 타입이 비연결성(UDP)이면 데이터그램이 전송되는 주소를 의미한다.
    - 만약 소켓 타입이 연결성(TCP) 이면 이 주소로 연결을 시도 (예를 들면 3-way handshaking)한다.
- 주소 구조체의 크기
    - 두번째 인수로 입력된 구조체의 크기를 삽입한다. 두번째 인수로 삽입되는 구조체의 길이는 주소의 형식마다 바뀔 것이므로 길이도 집어넣어야 한다.
    - 보통 sizeof 연산자 (sizeof는 함수처럼 사용하지만 실제로는 함수가 아니라 연산자에 가깝자)를 이용해 주소 구조체의 크기를 인수로 삽입한다.

만약에 해당 주소에 성공적으로 연결하는 데 (혹은 주소를 set하지 못할경우) 문제가 있을 경우 마찬가지로 시스템 콜이므로 -1을 리턴하며 임의의 errno를 set한다.

연결에 성공한다면 read 혹은 write로 읽고 쓸 수 있는 파일 디스크립터를 리턴한다.

## bind (system call)

```c
int bind(int socket, const struct socketaddr *address, socklen_t address_len);

struct sockaddr {
	unsigned char sa_len;         /* total length */
	unsigned char sa_family;      /* [XSI] address family */
	char          sa_data[14];    /* [XSI] addr value (actually larger) */
};

struct sockaddr_in {
	__uint8_t       sin_len;
	sa_family_t     sin_family;
	in_port_t       sin_port;
	struct  in_addr sin_addr;
	char            sin_zero[8];
};
```

bind 시스템 콜은 생성된 소켓에 대해 주소와 포트를 부여해 소켓만의 주소를 가질 수 있도록 해 준다.

서버 - 클라이언트 간 서버 역할을 하려면 해당 시스템 콜로 소켓에 대해 서버로 동작할 주소와 포트를 부여해야 한다.

인수는 세가지이며 소켓 파일 디스크립터, 주소 구조체, 주소 구조체의 길이를 받는다.

인수의 역할은 위 connect와 동일하며 생략한다.

만약 바인딩에 성공할 경우 0을 리턴하고 실패할 경우 -1을 리턴하며 임의의 errno를 set 한다.

## listen (system call)

```c
int listen(int socket, int backlog);
```

소켓을 기반으로 한 연결을 받아들일 수 있도록 해 준다. 해당 시스템 콜을 사용하면 소켓으로 들어오는 연결 요청을 받아들일 수 있는 상테가 되며 동시에 받을 수 있는 연결 개수에 대해 제한을 둘 수 있다. (1:N 통신을 지원한다.)

인수는 두가지이며 소켓 파일 디스크립터와 백로그 (연결요청 큐의 길이)를 받는다.

- 소켓
    - 소켓의 파일 디스크립터를 받는다.
- 백로그
    - 백로그의 사전적인 뜻은 "밀린 일"을 뜻한다. manual page를 참조하면 listen 시스템 콜의 백로그는 연결에 대한 밀린 요청을 쌓아두는 큐의 길이를 의미한다. 만약 밀린 연결 요청들이 큐의 길이를 벗어나면 여기에 접속하고자 하는 클라이언트는 연결 거절을 당한다.

만약 정상적으로 연결 요청을 수신할 준비가 되면 0을 리턴하며 문제가 있을 경우 -1을 리턴라고 errno를 set 한다.

## accept (system call)

```c
int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
```

(소켓마다 동작이 다를 수 있지만 일반적인 TCP/IP 연결 기준으로) accept 시스템 콜은 큐에서 대기 중인 연결 요청을 하나 뽑아 이 연결에 대한 하나의 소켓 파일 디스크립터를 반환한다. 인수의 주소에는 연결을 요청한 클라이언트의 주소 정보가 들어간다. 만약 대기 중인 연결이 큐에 없고 소켓이 블록킹으로 마킹되어 있으면 연결이 들어올 때까지 해당 함수에서 블록킹한다. 소켓이 논블록킹이고 대기 중인 연결이 큐에 없으면 오류를 리턴한다.

인수로 입력된 소켓 파일 디스크립터와 accept가 리턴하는 소켓 파일 디스크립터는 구분되어야 한다.

인수는 세 가지를 받는다.

- 소켓
    - 인수의 소켓 파일 디스크립터는 socket 시스템 콜로 생성되어야 하고 bind 시스템 콜로 주소와 포트가 바인딩 되어야 하며 listen 시스템 콜로 패킷을 받을 준비가 되어야 한다.
- 주소 구조체의 포인터
    - 커뮤니케이션 계층의 연결 개체의 주소(연결을 요청한 클라이언트의 주소)에 대한 값으로 채워진다.
- 주소 구조체의 길이 포인터
    - 주소 구조체의 길이가 저장된다.

정상적으로 처리가 될 경우 파일 디스크립터를 리턴하며 문제가 있을 경우 -1을 리턴하며 errno를 set 한다.

## send (system call)

```c
ssize_t send(int socket, const void* buffer, size_t length, int flags);
```

UNIX 혹은 유사한 운영체제에선 소켓을 파일 디스크립터로 제어하므로 파일 디스크립터에 데이터를 쓰는 write() 시스템 콜을 써도 된다. 그런데 소켓 파일 디스크립터에 한정해 기능이 조금 더 추가된 send() 라는 시스템 콜을 사용할 수 있다.

위의 함수 프로토타입을 보면 write 시스템 콜과 유사함을 볼 수 있으며 flags 인수가 추가된 것 외엔 사용법이 write와 비슷하다. 만약 flags 인수가 0이라면 write 시스템 콜과 동일하게 동작한다.

인수는 네 가지를 받는다.

- 소켓
    - 읽고 쓸 준비가 되어 있는 소켓의 파일 디스크립터를 받는다.
- 버퍼
    - 전송 할 데이터의 포인터를 받는다.
- 데이터의 길이
    - 전송 할 데이터의 길이를 받는다.
- 플래그
    - 보내는 동작에 대해 추가적인 옵션을 설정한다. 옵션은 or 연산자로 엮을 수 있으며 OS마다 약간씩 다르지만 BSD 메뉴얼에는 두 가지 플래그를 제공한다.
    - MSG_OOB : 긴급한 데이터임을 의미한다. 이 플래그를 set 하면 TCP 헤더의 URG 플래그가 set 된다. 이런 경우 패킷의 순서에 상관없이 이 패킷을 먼저 처리하라는 것을 의미한다.
    - MSG_DONTROUTE : 라우팅 테이블을 참조하지 않고 데이터를 송신한다. 라우팅 테이블은 라우터에 존재한다. 특수한 경우에만 사용한다.

정상적으로 처리가 될 경우 전송에 성공한 데이터의 길이를 리턴하며 송신시에 문제가 있을 경우 -1을 리턴하며 errno를 set 한다

## recv (system call)

```c
ssize_t recv(int socket, void* buffer, size_t length, int flags);
```

UNIX 혹은 유사한 운영체제에선 소켓을 파일 디스크립터로 제어하므로 파일 디스크립터에서 데이터를 읽는 read() 시스템 콜을 써도 된다. 그런데 소켓 파일 디스크립터에 한정해 기능이 조금 더 추가된 recv() 라는 시스템 콜을 사용할 수 있다.

위의 함수 프로토타입을 보면 read 시스템 콜과 유사함을 볼 수 있으며 flags 인수가 추가된 것 외엔 사용법이 read와 비슷하다. 만약 flags 인수가 0이라면 read 시스템 콜과 동일하게 동작한다.

인수는 네 가지를 받는다.

- 소켓
    - 읽고 쓸 준비가 되어 있는 소켓의 파일 디스크립터를 받는다.
- 버퍼
    - 전송받을 데이터의 포인터를 받는다.
- 데이터의 길이
    - 전송 받을 데이터의 길이를 받는다.
- 플래그
    - 받는 동작에 대해 추가적인 옵션을 설정한다. 옵션은 or 연산자로 엮을 수 있으며 OS마다 약간씩 다르지만 BSD 메뉴얼에는 세 가지 플래그를 제공한다.
    - MSG_OOB : OOB 패킷을 받을 때 사용한다. 보통 OOB 패킷을 받으면 SIGURG 시그널이 발생하는데 OOB 패킷은 해당 플래그를 set 한 recv 함수로 처리한다.
    - MSG_PEEK : 보통 recv나 read 시스템 콜로 데이터를 읽으면 큐에 데이터가 뽑히게 된다. 이 데이터를 그대로 유지시키고 싶을 때 해당 플래그를 set한다.
    - MSG_WAITALL : 수신하고자 하는 데이터가 버퍼에 찰 때까지 블록한다.

## getsockopt / setsockopt (system call)

```c
int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname, void *optval, socklen_t optlen);
```

네트워크 환경은 다소 복잡하고 다양하여 예측하기 힘든 상황이 발생할 수 있으므로 세부사항을 조절해야하는 경우가 있다. 따라서 소켓의 세부사항을 조절하는 함수가 필요하다.

위의 두 함수는 소켓의 세부 사항을 설정하는 함수이다.

getsockopt은 소켓의 설정을 가져올 때 호출하며 setsockopt은 소켓의 설정을 세팅할 때 호출한다.

인수는 5개를 받는다.

- sockfd
    - 옵션 확인 및 설정을 위한 소켓의 파일 디스크립터를 전달한다.
- level
    - 다루고자 하는 옵션의 프로토콜 레벨을 설정한다.
    - SOL_SOCKET, IPPROTO_TCP 둘 중 하나를 사용한다.
- optval
    - 확인 결과를 저장하기 위한 버퍼의 주소값을 전달한다.
    - 상황에 따라 boolean 자료형이나 int 자료형을 사용하므로 `void *` 자료형으로 optval 값을 넘긴다.
- optlen
    - 옵션 정보(optval)의 크기를 의미한다.
    - 이는 함수가 호출될 때, 바이트 단위로 계산되어 저장된다. (set 함수에서는 포인터를 받는다.)

설정하고자 하는 옵션은 다음과 같은 것들이 있다.

- SO_SNDBUF: 전송소켓에서 사용할 최대 버퍼(출력버퍼)의 크기를 설정한다.
- SO_RCVBUF: 수신소켓에서 사용할 최대 버퍼(출력버퍼)의 크기를 설정한다.
- SO_REUSEADDR: bind로 할당된 소켓 자원을 프로세스가 재사용 할 수 있도록 허락해주는 옵션이다. 프로그램을 재시작시킬 경우 일정시간을 기다려야하는 번거로움이 있기 때문에 이를 막을 수 있다. (그 안에 다시 사용하면 `bind error: Address already in use`라는 문구가 출력됨)
- SO_LINGER: 소켓이 `close`되었을 때, 소켓 버퍼에 남아있는 데이터를 어떻게 처리할 것인지 결정한다.
    
    ```
    struct linger
    {
      int l_onoff;  // linger 옵션을 끌 것인지 켤 것인지 결정
      int l_linger; // 기다리는 시간을 결정
    }
    ```
    
    l_onoff == 0: linger의 영향을 받지 않으며, 소켓의 기본설정으로 소켓 버퍼에 남아있는 모든 데이터를 보낸다. `close`는 바로 리턴을 한다.
    
    l_onoff > 0, l_linger == 0: hard 혹은 abortive 종료이다. `close`는 바로 리턴을 하며 소켓 버퍼에 아직 남아있는 데이터는 버린다. TCP 연결 상태에서 상대편 호스트에 리셋을 위한 RST 패킷을 보낸다.
    
    l_onoff > 0, l_linger > 0: 버퍼에 남아있는 데이터를 모두 보낸다. `close`에서는 l_linger에 지정된 시간만큼 블럭상태에서 대기한다. 시간 내에 데이터를 모두 보내지 못하면 에러가 리턴된다.
    
- SO_KEEPALIVE: 세션이 끊어졌는지 끊어지지 않았는지 확인을 위해서 패킷을 보내서 체크해 준다. 좀비세션을 막기 위해 사용되기도 한다. 주기적으로 확인 패킷을 보내는데 응답이 없으면 잘못된 소켓이라고 판단하여 소켓을 종료할 수 있다. (좀비세션: 서버나 클라이언트가 정상적으로 종료하지 않았음에도 불구하고 종료되어 서버나 클라이언트는 세션의 안녕을 확인하지 못하는 상태)
- SO_TYPE: 소켓 타입은 소켓 생성시 한 번 결정되면 변경이 불가능 하며, get 함수에서만 사용하는 옵션이다. 함수에는 없다. `SOCK_STREAM`은 TCP 소켓임을, `SOCK_DGRAM`은 UDP 소켓임을 의미한다.

# 소켓 보조 함수

## 바이트 재정렬 함수 (표준 라이브러리 함수)

- 네트워크 패킷의 바이트 정렬은 Big Endian을 따르지만 CPU마다 데이터의 저장 순서가 달라 (일반적인 PC는 Little Endian 순서로 데이터를 저장한다.) 바이트를 재정렬 할 필요가 있다.

### Host TO Network byte order (hton 함수 계열)

```cpp
#include <arpa/inet.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
```

- 바이트의 길이에 따라 함수 끝에 l(long), s(short)로 구분된다.
- 주로 포트 번호를 address 구조체에 넣을 때 사용한다.

### Network TO Host byte order (ntoh 함수 계열)

```cpp
#include <arpa/inet.h>

uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
```

- 바이트의 길이에 따라 함수 끝에 l(long), s(short)로 구분된다.
- 주로 address 구조체에 들어있는 포트 번호를 받을 때 사용한다.

## 주소 변환 함수

- .과(IPv6의 경우엔 :) 숫자로 이루어져 있는 IP 주소의 문자열을 실제 숫자로 파싱해준다. (atoi와 비슷하다.)

### inet_addr

```cpp
in_addr_t inet_addr(const char *cp);
```

- IPv4 양식의 주소 문자열(Dot-decimal notation)을 실제 숫자로 변환해 준다.
- 변환 실패 시 INADDR_NONE (-1)을 리턴한다.