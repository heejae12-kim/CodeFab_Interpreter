# CodeFab Interpreter — 언어 사용 설명서

CodeFab Interpreter가 지원하는 언어 및 shell 사용 설명서.

---

## 문법 목차

1. [기본 규칙](#1-기본-규칙)
2. [리터럴 & 타입](#2-리터럴--타입)
3. [연산자](#3-연산자)
4. [변수](#4-변수)
5. [출력](#5-출력)
6. [제어 흐름](#6-제어-흐름)
7. [블록 & 스코프](#7-블록--스코프)
8. [함수](#8-함수)
9. [배열](#9-배열)
10. [오류 유형](#10-오류-유형)

## Shell 목차
1. [실행 방법](#1-실행-방법)
2. [디버그](#2-debug)

---
# 문법

## 1. 기본 규칙

- 모든 **문(statement)** 은 세미콜론 `;` 으로 끝나야 합니다. 누락 시 `ParseError` 발생.
- **주석** 은 one-line만 지원하며 `//` 로 시작하며 줄 끝까지 무시됩니다.
- 식별자(변수·함수 이름)는 영문자 또는 `_` 로 시작하고, 이후 영문자·숫자·`_` 를 사용할 수 있습니다.
- 키워드는 **대소문자를 구분**합니다. 특히 `Func`, `Array` 는 반드시 **대문자로 시작**해야 합니다.

```
// 이것은 주석입니다
var x = 42;    // 세미콜론 필수
```

---

## 2. 리터럴 & 타입

| 타입 | 리터럴 예시 | 설명 |
|------|-----------|------|
| `number` | `42`, `3.14`, `-7` | 배정밀도 부동소수점. 정수값이면 소수점 없이 출력 |
| `string` | `"hello"`, `""` | 큰따옴표로 감쌈 |
| `bool` | `true`, `false` | 불리언 |
| `nil` | *(소스에 직접 쓸 수 없음)* | 초기화되지 않은 변수, `return;`, 빈 배열 요소의 값 |
| `array` | `Array(n)` | 크기 n의 배열. 내장 함수 `Array`로만 생성 가능 |

### 주의: nil은 리터럴이 아닙니다

`nil` 을 소스 코드에 직접 작성할 수 없습니다. 아래 방법으로만 `nil` 값을 얻을 수 있습니다.

```
var x;           // x는 nil
var y = Array(3);
print y[0];      // nil  (배열 초기 요소)
```

```
// ❌ ParseError: nil은 예약어가 아니므로 식별자로 해석됨
var a = nil;
```

### 진리값 규칙

`false` 와 `nil` 만 거짓(falsy)입니다. **숫자 `0`, 빈 문자열 `""` 은 참(truthy)** 입니다.

| 값 | 진리값 |
|----|--------|
| `false` | 거짓 |
| `nil` | 거짓 |
| `true` | 참 |
| `0`, `3.14` 등 모든 숫자 | 참 |
| `""`, `"hello"` 등 모든 문자열 | 참 |
| 배열 | 참 |

---

## 3. 연산자

### 산술 연산자

| 연산자 | 허용 피연산자 | 예시 |
|--------|------------|------|
| `+` | 숫자+숫자, **문자열+문자열** | `1 + 2` → `3`, `"a" + "b"` → `"ab"` |
| `-` | 숫자+숫자만 | `5 - 3` → `2` |
| `*` | 숫자+숫자만 | `4 * 3` → `12` |
| `/` | 숫자+숫자만, **0 나눗셈 금지** | `10 / 2` → `5` |

**`+` 사용 조건 (엄격)**

```
// ✅ 허용
var a = 1 + 2;          // 숫자 + 숫자
var b = "a" + "b";      // 문자열 + 문자열

// ❌ RuntimeError: 숫자와 문자열 혼합
var c = 1 + "a";
var d = "a" + 1;
```

**`-`, `*`, `/` 사용 조건 (엄격)**

```
// ❌ RuntimeError: 문자열에 산술 연산 불가
var x = "hello" - 1;
var y = "hello" * "world";
var z = "hello" / 2;

// ❌ RuntimeError: 단항 - 를 문자열에 적용 불가
var w = -"hello";

// ❌ RuntimeError: 0 나눗셈
var v = 10 / 0;
```

### 비교 연산자

| 연산자 | 허용 피연산자 | 설명 |
|--------|------------|------|
| `==` | 모든 타입 쌍 | 같음. 타입이 다르면 항상 `false` |
| `!=` | 모든 타입 쌍 | 다름. 타입이 다르면 항상 `true` |
| `<` | 숫자+숫자만 | 미만 |
| `<=` | 숫자+숫자만 | 이하 |
| `>` | 숫자+숫자만 | 초과 |
| `>=` | 숫자+숫자만 | 이상 |

```
// ✅ 허용
print 3 == 3;        // true
print "a" == "a";    // true  (문자열 내용 비교)
print 3 == "3";      // false (타입이 다름 — 오류 아님)
print 1 < 2;         // true

// ❌ RuntimeError: < <= > >= 에 숫자 외 사용 불가
print "a" < "b";
print true > false;
print 1 > "2";
```

### 논리 연산자

| 연산자 | 설명 |
|--------|------|
| `and` | 논리 AND |
| `or` | 논리 OR |
| `!` | 논리 NOT |


```
// ✅ 허용
true and false      // false
false or true       // true
!true               // false
!nil                // true

// ❌ ParseError: && || 는 지원 안 함
true && false
true || false
```

### 연산자 우선순위 (높음 → 낮음)

| 순위 | 연산자 |
|------|--------|
| 1 | `!` (단항) |
| 2 | `*`, `/` |
| 3 | `+`, `-` |
| 4 | `<`, `<=`, `>`, `>=`, `==`, `!=` |
| 5 | `and` |
| 6 | `or` |
| 7 | `=` (대입, 오른쪽 결합) |

**괄호 `( )` 로 우선순위를 명시적으로 지정할 수 있습니다.**

---

## 4. 변수
자료형을 명시하지 않아도 됩니다.

### 선언

```
var 이름;               // nil 로 초기화
var 이름 = 표현식;
```

```
var x;              // nil
var y = 10;
var msg = "CodeFab";
var flag = true;
```

### 대입

선언 이후 `=` 로 값을 변경합니다. 대입 대상은 반드시 **변수 식별자** 또는 **배열 인덱스 식** 이어야 합니다.

```
var a = 1;
a = a + 1;          // ✅ 변수 대입

var arr = Array(3);
arr[0] = 99;        // ✅ 배열 요소 대입

// ❌ RuntimeError: 선언되지 않은 변수에 대입
x = 5;

// ❌ RuntimeError: 선언되지 않은 변수 읽기
print y;

// ❌ ParseError: 유효하지 않은 대입 대상
a + 1 = 10;
```

### 스코프 규칙

- 변수는 선언된 블록 `{ }` 안에서만 유효합니다.
- 블록 밖에서 선언된 변수는 블록 안에서도 읽고 쓸 수 있습니다.
- `for` 루프의 `var i` 는 루프 블록 내에서만 유효합니다.

---

## 5. 출력

`print` 뒤에 표현식을 쓰면 결과를 한 줄로 출력합니다.

```
print 42;           // 42
print 3.0;          // 3  (정수값이면 소수점 없이 출력)
print "hello";      // hello
print true;         // true
print false;        // false

var x;
print x;            // nil

var arr = Array(3);
arr[0] = 10;  arr[1] = 20;  arr[2] = 30;
print arr;          // [10, 20, 30]
print arr[0];       // 10
```

---

## 6. 제어 흐름

### if / else

```
if (조건) 문
if (조건) 문 else 문
```

- 조건식은 **반드시 괄호 `( )` 로 감싸야** 합니다. 누락 시 `ParseError`.
- 조건식은 **비어 있을 수 없습니다** (`if ()` → `ParseError`).
- 본문(문)은 단일 statement 또는 블록 `{ }` 모두 허용합니다.
- chained `if-else` 사용 가능합니다.

```
var x = 5;

// ✅ 블록 본문
if (x > 3) {
	print "big";
}

// ✅ 단일 statement 본문
if (x > 3) print "big"; else print "small";

// ✅ else if 패턴 (else 뒤에 if 중첩)
if (x < 0) {
	print "negative";
} else if (x == 0) {
	print "zero";
} else {
	print "positive";
}

// ❌ ParseError: 조건 괄호 없음
if x > 3 { print "big"; }

// ❌ ParseError: 조건식 없음
if () { print "x"; }
```

### for 루프

```
for (초기화; 조건; 증감) 문
```

세 절 모두 생략 가능하지만, **세미콜론 2개는 반드시 있어야** 합니다.

| 생략 | 의미 |
|------|------|
| 초기화 생략 | 루프 전 외부에서 변수를 미리 선언 |
| 조건 생략 | 무한 루프 (종료 방법 없으면 프로그램이 멈추지 않음) |
| 증감 생략 | 본문 안에서 직접 증가·감소 처리 |

```
// ✅ 기본 카운터 루프
for (var i = 0; i < 5; i = i + 1) {
	print i;
}

// ✅ 초기화 생략 (외부 변수 사용)
var n = 0;
for (; n < 3; n = n + 1) {
	print n;
}

// ✅ 증감 생략 (본문 안에서 처리)
for (var i = 0; i < 3;) {
	print i;
	i = i + 1;
}

// ❌ ParseError: 첫 번째 세미콜론 누락
for (var i = 0 i < 5; i = i + 1) { print i; }

// ❌ ParseError: 두 번째 세미콜론 누락
for (var i = 0; i < 5 i = i + 1) { print i; }

// ❌ ParseError: 괄호 없음
for var i = 0; i < 5; i = i + 1 { print i; }
```

---

## 7. 블록 & 스코프

중괄호 `{ }` 로 블록을 만들고, 내부에서 선언한 변수는 블록 밖에서 접근할 수 없습니다.

```
{
	var inner = 10;
	print inner;    // 10
}
// print inner;    // ❌ RuntimeError: inner는 여기서 존재하지 않음
```

내부 스코프에서 같은 이름의 변수를 선언하면 외부 변수를 **가립니다(shadowing)**. 블록이 끝나면 외부 변수가 복원됩니다.

```
var a = 1;
{
	var a = 2;
	print a;    // 2
}
print a;        // 1 (외부 변수 복원)
```

---

## 8. 함수

### 선언

> **주의**: 키워드는 `Func` 입니다. 소문자 `func` 는 `ParseError`.

```
Func 함수명(매개변수, ...) {
	// 본문
	return 표현식;
}
```

```
Func add(a, b) {
	return a + b;
}

print add(3, 7);    // 10
```

### 반환값

| 작성 방법 | 반환 결과 |
|---------|---------|
| `return 표현식;` | 표현식의 값 |
| `return;` | `nil` |
| `return` 없이 함수 끝 | `nil` |

### 인자 수

선언된 매개변수 수와 호출 시 인자 수가 **정확히 일치**해야 합니다. 불일치 시 `RuntimeError`.

```
Func add(a, b) { return a + b; }

// ❌ RuntimeError: Expected 2 arguments but got 3.
add(1, 2, 3);

// ❌ RuntimeError: Expected 2 arguments but got 1.
add(1);
```

### 함수 중복 정의 & Overloading

동일한 이름으로 함수를 두 번 정의하면 에러가 발생합니다.
이름이 같고 매개변수 수가 달라도 마찬가지입니다(**Overloading 미지원**).

```
Func add(a, b) { return a + b; }
Func add(a, b, c) { return a + b + c; }

// add는 현재 (a, b, c) 버전으로 덮어써짐
// ❌ RuntimeError: Expected 3 arguments but got 2.
print add(1, 2);
```

### 재귀

```
Func fact(n) {
	if (n <= 1) return 1;
	return n * fact(n - 1);
}

print fact(5);    // 120
```

함수는 선언 시점의 환경을 캡처합니다(렉시컬 스코프). 변수에 담거나 다른 함수의 인자·반환값으로 사용할 수 있습니다.

```
Func makeCounter() {
	var count = 0;
	Func inc() {
		count = count + 1;
		return count;
	}
	return inc;
}

var c = makeCounter();
print c();    // 1
print c();    // 2
```

### 비함수 값 호출

함수가 아닌 값을 `( )` 로 호출하면 `RuntimeError`.

```
var x = 10;
x();    // ❌ RuntimeError: Can only call functions.
```

---

## 9. 배열

### 생성

> **주의**: 키워드는 `Array` (대문자 A) 입니다. 소문자 `array` 는 `ParseError`.

```
var 변수 = Array(크기);
```

```
var arr = Array(5);     // 크기 5짜리 배열, 모든 요소는 nil
```

**생성 조건 (엄격)**

- 크기는 반드시 **숫자 표현식** 이어야 합니다.
- 정수가 아닌 값(문자열, bool 등)을 크기로 넣으면 `RuntimeError`.

```
var a = Array(3);       // ✅
var b = Array(2 + 3);   // ✅ 숫자 표현식

// ❌ RuntimeError: 크기에 문자열 사용
var c = Array("hello");

// ❌ RuntimeError: 크기에 bool 사용
var d = Array(true);
```

### 읽기 & 쓰기

인덱스는 **0-based 정수** 입니다.

```
var arr = Array(3);
arr[0] = "a";
arr[1] = "b";
arr[2] = "c";

print arr[0];       // a
print arr;          // [a, b, c]
```

**인덱스 조건 (엄격)**

| 상황 | 결과 |
|------|------|
| 초기 요소 읽기 | `nil` |
| 정상 범위 접근 (0 이상 size 미만) | 해당 값 |
| 인덱스 ≥ size 또는 인덱스 < 0 | `RuntimeError` |
| 인덱스가 숫자가 아님 (문자열, bool 등) | `RuntimeError` |
| 배열이 아닌 값에 `[ ]` 사용 | `RuntimeError` |

```
var arr = Array(3);
arr[0] = 10;

print arr[0];       // ✅ 10
print arr[1];       // ✅ nil (초기값)

// ❌ RuntimeError: 범위 초과
print arr[5];

// ❌ RuntimeError: 문자열 인덱스
print arr["hello"];

// ❌ RuntimeError: 배열이 아닌 값 인덱싱
var x = 10;
print x[0];
```

### 배열 순회

```
var arr = Array(4);
for (var i = 0; i < 4; i = i + 1) {
	arr[i] = i * 2;
}
for (var i = 0; i < 4; i = i + 1) {
	print arr[i];    // 0, 2, 4, 6
}
```

---

## 10. 오류 유형

| 오류 | 발생 시점 | 주요 원인 |
|------|---------|---------|
| `LexError` | 어휘 분석 | 인식할 수 없는 문자, 닫히지 않은 문자열 |
| `ParseError` | 구문 분석 | 세미콜론 누락, 괄호·중괄호 누락, 잘못된 식 구조 |
| `RuntimeError` | 실행 중 | 타입 불일치, 미정의 변수, 0 나눗셈, 배열 범위 초과, 인자 수 불일치 등 |

오류 메시지에는 발생한 **줄 번호**가 포함됩니다.

### 자주 혼동하는 오류 사례

```
// ❌ ParseError: nil 리터럴 없음
var a = nil;

// ❌ ParseError: && || 없음, and or 사용
if (x > 0 && y > 0) { ... }

// ❌ ParseError: func 는 소문자 불가
func foo() { ... }

// ❌ ParseError: array 는 소문자 불가
var arr = array(3);

// ❌ RuntimeError: 숫자 + 문자열
var r = 1 + "a";

// ❌ RuntimeError: 문자열 대소 비교
if ("a" < "b") { ... }
```

---

## 빠른 참조 — 키워드 목록

| 키워드 | 용도 |
|--------|------|
| `var` | 변수 선언 |
| `if` / `else` | 조건 분기 |
| `for` | 반복 루프 |
| `print` | 출력 |
| `Func` | 함수 선언 |
| `return` | 함수 반환 |
| `Array` | 배열 생성 |
| `true` / `false` | 불리언 리터럴 |
| `and` / `or` | 논리 연산자 |

---

## 1. 실행 방법
Shell을 통해 CLI로 코드를 line 단위 실행, unit test 실행, 또는 파일 입력으로 실행을 할 수 있음.

| 옵션 | 설명 | 비고 |
| ---- | ---- | ---- |
| - | 프롬프트를 통해 코드를 입력 받고 1줄 단위로 실행함. | - |
| `unit` | Unit Test를 실행함. | - |
| `run <file_path>` | 파일을 입력받아 코드를 실행함. | 입력할 파일의 경로를 반드시 입력해야 함. **파일 형식은 `*.txt`를 권장함.** |
| `debug` | 파일을 입력 받고 디버그 모드로 실행함. | line 단위 수행, break, 변수 관찰 등을 할 수 있음. |


## 2. Debug
### 1) Stepping

statement 단위로 파이프라인을 정지하며 단계별로 실행할 수 있음.
| 사용법 | 설명 | 비고 |
| --- | --- | ---|
| `step` | 현재 statement 실행 후 다음 statement에서 정지 |  |
| `next` | 현재 statement 실행 | 블록 내부로 진입 X |
| `break <line>` | 해당 `line`에 breakpoint 설정 | |
| `Breakpoints` | 현재 설정된 breakpoints 목록 출력 |  |
| `remove <line>` | `line`의 breakpoint 해제 |  |
| `continue` | 다음 breakpoint까지 실행 |  |

### 2) Watch Variables

실행 중인 step 단위로 변수 값을 확인할 수 있음.
|사용법 | 설명 | 비고 |
| --- | --- | --- |
| `watch <변수명>` | 변수를 관찰 목록에 추가할 수 있음 | TODO) _같은 이름을 가진 local, global 변수가 있으면 어떡하지?_ |
| `unwatch <변수명>` | 관찰 목록에서 변수를 제거할 수 있음. |  |
| `watches` | 현재 감시 중인 변수 목록과 값 출력함. |  |
| `inspect` | 현재 scope에서 모든 변수 출력함. | `[local], [global]` 표시됨. |