# Synchronous / Asynchronous

두 단어의 뜻은 동기와 비동기라는 뜻이다.

위의 두 단어는 상세하게 다루면 여러 의미로 폭넓게 다루지만 프로그래밍에서 무엇인가를 호출할 때, 호출한 결과를 기다려 결과값을 받을지, 혹은 기다리지 않고 즉시 결과값을 받을지에 대한 차이이다.

예를 들어 어떤 기능 (주로 시스템 콜)을 호출하였는데 이 호출 결과를 기다릴지, 아니면 호출 결과를 기다리지 않고 다름 코드 블럭을 실행할지에 대한 차이가 된다.

동기와 비동기는 블록킹과 논블록킹과 혼동하기 쉽다.

## 동기

단순하게 사용자로부터 쉘을 통해서 아래처럼 입력을 받는 프로그램을 만든다고 생각해 보자.

```python
str = input("무엇인가를 입력해 보세요.")
print(str)
```

이 경우 프로그램의 흐름은 **표준 입력으로부터 입력을 받을 때까지** 프로그램의 흐름은 멈춰 있는다.

이런 방식의 일반적인 프로그램을 동기 방식이라고 한다.

이런 방식은 하는 일이 단순하거나 반드시 앞에 어떤 일이 진행되고 나서 다음 결과가 진행되어야 할 때의 일반적인 구조이다.

하지만 여러 클라이언트로부터 요청을 받는 웹 서버와 같은 프로그램이 클라이언트들의 요청을 하나씩 실행한다면 문제가 될 수 있다.

또 동시에 여러 요소가 입력되고 출력되는 게임같은 프로그램을 한 프로그램, 한 스레드의 문맥에서 처리한다면 게임이 끊겨 보일 것이다.

## 비동기

코드를 말 그대로 비동기적으로 실행하는 것을 의미한다. 이런 경우 코드 A와 코드 B를 동시에 실행하거나 특정 입력을 기다리는 동안 다른 일을 할 수도 있다.

만약에 위 input 함수가 비동기적으로 동작된다고 하면 사용자로부터 입력을 받기 전에 무엇인가가 리턴될 것이고 곧바로 리턴된 값을 출력하게 된다.

비동기 프로그래밍은 여러 프로그래밍 환경에서 스레드, 코루틴 등으로 여러 형태와 방법으로 지원한다.

### Apache Tomcat의 비동기 프로그래밍 예시

아파치 톰캣은 자바 기반으로 서버 개발을 할 때 사용하는 WAS 중 하나이다.

Tomcat 3.2 이전 버전에서는 클라이언트에게 요청이 오면 요청 하나당 하나의 스레드를 생성하여 스레드 내에서 요청을 처리하였다. 이런 식으로 구현하면 클라이언트에 대한 요청이 바로 실행되는 것처럼 동작될 것이다.

하지만 요청 하나당 하나의 스레드를 사용하는 것은 비효율적이다. 일반적으로 스레드를 생성하고 제거하는 데엔 프로세스보다는 적지만 비교적 무거운 작업이기 때문에 스레드로 인한 오버헤드가 생긴다.

그래서 Tomcat 3.2버전 이후로는 일정 개수의 스레드를 미리 만들어 놓고 요청이 들어오면 이 요청을 스레드가 별도로 처리하게 만든다. 이를 스레드 풀이라고 한다.