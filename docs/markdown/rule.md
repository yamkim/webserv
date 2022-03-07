# 코딩 컨벤션

> 💡 읽기 쉬운 코딩 컨벤션을 지향합니다.

# tab or space

들여쓰기 시 4 space 길이의 1개의 탭 사용을 원칙으로 합니다.

# 네이밍 컨벤션

## 철학

일관성이 있고 직관적이며 이름이 다소 길어지더라도 적절한 이름을 사용하여 코드 파악을 쉽게 하는것을 목표로 삼습니다.

과도한 약어는 지양하지만 개발자들끼리 암묵적으로 줄여쓰는 약어 (예 - file descriptor : fd)는 약어를 사용합니다.

## 변수

### 클래스의 멤버 변수

- 클래스의 멤버 변수에는 앞에 언더바 1개 (_)를 붙이는 것을 원칙으로 합니다.
- 특별한 규칙이나 약어가 없을 경우 단어와 단어들을 조합해 변수명을 지을 때에는 Lower Camel 표기법을 사용합니다.
- 예제는 다음과 같습니다.

```cpp
class Test {
	int _fd; // file descriptor
	int _count; // Count
	char *_className; // Class name
	long _classGenereteTime; // Class Generete Time
}
```

### 함수의 지역 변수

- 클래스의 멤버 변수 명명법에 언더바가 빠진 것과 동일합니다.
- 특별한 규칙이나 약어가 없을 경우 단어와 단어들을 조합해 변수명을 지을 때에는 Lower Camel 표기법을 사용합니다.
- 예제는 다음과 같습니다.

```cpp
void func (void) {
	int fd; // file descriptor
	int count; // Count
	char *className; // Class name
	long classGenereteTime; // Class Generete Time
}
```

### 전역 변수

- 전역 변수임을 나타내는 g_를 변수명 앞에 붙입니다.
- 특별한 규칙이나 약어가 없을 경우 단어와 단어들을 조합해 변수명을 지을 때에는 Lower Camel 표기법을 사용합니다.
- 예제는 다음과 같습니다.

```cpp
int g_fd; // file descriptor
int g_count; // Count
char *g_className; // Class name
long g_classGenereteTime; // Class Generete Time
```

### static 변수

- 다른 변수와 구별될 필요가 있으므로 static 변수임을 나타내는 s_를 변수명 앞에 붙입니다.
- 모호함을 방지하기 위해 static 변수는 선언과 동시에 반드시 초기화를 시킵니다.
- 특별한 규칙이나 약어가 없을 경우 단어와 단어들을 조합해 변수명을 지을 때에는 Lower Camel 표기법을 사용합니다.
- 예제는 다음과 같습니다.

```cpp
int s_fd = -1; // file descriptor
int s_count = 0; // Count
char *s_className = NULL; // Class name
long s_classGenereteTime = 123456789; // Class Generete Time
```

### 리터럴 할당

- 변수에 값을 할당시, 숫자 리터럴을 직접적으로 할당하는 것을 지양합니다.
    - 전처리 시점에서 넣고자하는 값을 문자로 치환합니다.
    - 모두 대문자로 작성하되, under bar로 연결합니다.(#define 이용)
    
    ```c
    myIntArray[10] = { 0, }; // X
    
    #define INT_ARRAY_SIZE 10
    myIntArray[INT_ARRAY_SIZE] = { 0, }; // O
    ```
    

# 클래스

## 정의

- 클래스 이름은 Pascal 표기법을 사용합니다.
- 클래스의 선언은 hpp 파일에서 하며 클래스의 멤버 함수에 대한 구현은 cpp 파일에서 합니다.
    - 단, 템플릿의 경우는 예외로 하며 모든 선언과 기능 구현을 hpp에서 하는 것을 원칙으로 합니다.
- 브라켓 ({})은 자바(스크립트) 스타일과 유사하게 합니다.
- 접근 지정자 (private, public, protected)는 클래스 선언으로부터 한 탭 들여쓰며 접근 지정자의 예하 코드는 접근 지정자로부터 한 탭 들여씁니다.

```cpp
class ClassName {
	private:
		int val;
}
```

# 함수

## 정의

- 함수의 명칭은 Lower Camel 표기법을 사용합니다.
- 이는 클래스의 멤버 함수에도 적용됩니다.
- static 함수의 경우엔 추가로 s_ 키워드를 붙입니다.
- 함수 작성시 동사를 먼저 적으며 줄임말을 사용하지 않습니다.

```cpp
int getTimeOfDayCustom(void);
int s_testStaticFunction();
int openSocket() { ... }
int cntArrSize() { ... } // X
int countArraySize(){ ... } // O
```

## return

- return 할 대상을 괄호로 감쌉니다.
- 예를 들면 return (rtn);

### 함수로만 이루어진 파일

- 하나의 소스코드(*.cpp)에서 최종적으로 노출되는 함수는 하나로 제한합니다.

```c
static int part1() { ... }
static int part2() { ... }

int openSocket() {
	...
}
```

# 제어문

## if문

### `if - elseif - else`

- `if - elseif - else`는 다음과 같이 적습니다.

```cpp
if (condition_1 == true) {
	// code 1
} else if (condition_2 == -1) {
	// code 2
} else {
	// code 3
}
```

### `if (condition)`

- 시스템 콜 호출이 많은 과제인만큼 최대한 condition을 명확하게 적는 것을 지향합니다.
- condition이 길어지면 최대한 가독성이 좋게 합니다. (각자 재량에 맞게)

### `{}`

- 조건문 예하에 명령문이 한 줄이어도 브라켓을 적는 것을 원칙으로 합니다.

## 사용 지양 제어문

- `goto` 문
- `switch` 문 (가능한한 `if - elseif - else` 문 사용)

# 반복문

## `for`

- 초기식은 불가피한 경우가 아니라면 초기식 내에서만 선언합니다.
- 증감식은 아래와 같은 형태로 적는 것을 원칙으로 합니다.

```c
for (int i = 0; i < range; ++i) {
	// do something...
}
```

## `while`

- `break` 조건을 반드시 `while` 괄호 내에 적을 필요는 없습니다.

## `do-while`

- `do-while`문은 사용하지 않습니다.