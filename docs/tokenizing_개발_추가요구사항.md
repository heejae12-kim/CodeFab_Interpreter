# tokenizing 개발 문서 — 추가 요구사항 (3일차)

> **규칙**: 이 문서는 3일차 추가 요구사항 구현 히스토리를 기록한다.  
> 기존 개발 문서(`tokenizing_개발.md`)의 세션 009 이후 작업을 별도 파일로 관리한다.

---

## 목차

- [세션 010 — 3일차 추가 요구사항 분석 (2026-06-05)](#세션-010--3일차-추가-요구사항-분석-2026-06-05)
- [세션 011 — 함수 지원 Token/Lexer 수정 (2026-06-05)](#세션-011--함수-지원-tokenlexer-수정-2026-06-05)
- [세션 012 — 논리 연산자 Token/Lexer 수정 (2026-06-05)](#세션-012--논리-연산자-tokenlexer-수정-2026-06-05)

---

## 세션 010 — 3일차 추가 요구사항 분석 (2026-06-05)

### 커밋
```
(문서 작업 — 별도 커밋 없음)
```

### 사용자 프롬프트 원문
```
새로운 요구사항이 추가가 되었어, 함수를 지원해야 하고, 변수 타입에 대해 변수도
추가가 되어야 해. 이에 대한 요구 사항은 @"3일차_CodeFab Interpreter.pdf" 에
자세히 설명되어 있어. chapter 2 와 chapter 3 이야. 이 요구사항을 만족하기 위해
현재 우리 코드가 어떻게 바뀌어야 하는지 검토부터 해줄래?
```

```
우선 지금 분석한 내용을 문서로 먼저 작성해줘.
문서 이름은 "추가요구사항분석_chapter_2_and_3" 으로
현재 디렉토리 기준으로 개발 문서 위치에 저장해줘.
```

### 주목적
PDF Chapter 2(함수), Chapter 3(정적 배열) 요구사항을 파악하고 전체 파일별 변경 분석을 문서화한다.

### 분석 결과 요약

#### Chapter 2 — 함수
| 기능 | 예시 |
|------|------|
| 함수 선언 | `Func add(a, b) { return a + b; }` |
| 함수 호출 | `ret = add(3, 7);` |
| return 없는 함수 | null 반환 |
| 재귀 호출 | `Func fact(n) { if(n<=1) return 1; return n * fact(n-1); }` |

정적 오류: 함수 외부 return, 파라미터 이름 중복, 비함수 호출, 인자 개수 불일치

#### Chapter 3 — 정적 배열
| 기능 | 예시 |
|------|------|
| 배열 생성 | `var arr = Array(3);` |
| 인덱스 쓰기 | `arr[0] = 10;` |
| 인덱스 읽기 | `print arr[0];` |

런타임 오류: 범위 초과, 비숫자 인덱스, 비배열 대상, 비숫자 크기

### 생성된 문서
| 파일 | 내용 |
|------|------|
| `docs/추가요구사항분석_chapter_2_and_3.md` | 전체 파일별 변경 명세 (7개 파일) |

### 다음 세션 목표
Tokenizing 담당 범위(Token.h, Lexer.cpp)의 함수 지원 요구사항 분석 및 구현.

---

## 세션 011 — 함수 지원 Token/Lexer 수정 (2026-06-05)

### 커밋
```
1a49036  [feat] Add FUNC, RETURN, COMMA token types for function support
```

### 사용자 프롬프트 원문
```
지금 내가 이 프로젝트에서 맡고 있는 역할은 tokenizing 부분이야. lexer 까지도
내가 수정해야 하는 부분이고, 새로운 요구사항에 맞춰서 새로 정의가 되어야 하는
token 을 지원하도록 수정해야 돼. 우선 요구사항 분석 먼저 진행해주고, function
지원을 위한 token 부분만 먼저 어떻게 지원해야 하는지 판단해주고 문서로 먼저
작성해줄래?
```

```
이름을 tokenizing_추가요구사항분석_function 이라는 형식으로 저장해주고,
작업을 진행하면서 주석 및 각 단계별로 commit 을 나누어서 진행해줘.
파일들은 어떻게 할 것인지는 나에게 물어보고 진행해줘.
```

```
lexer 수정 부분도 진행하고 난 다음에 판단하고 커밋할지 이야기 해줄께,
다음 단계로 진행하자
```

```
빌드 및 기존 테스트 진행해
```

```
지금 커밋 한번 하자. 제목만 [feat] 을 붙여서 진행해줘
```

### 주목적
함수 선언(`Func`), 반환문(`return`), 파라미터 구분자(`,`)를 Lexer가 인식하도록 Token/Lexer를 수정한다.

### 분석 포인트 — `Func` 대소문자 결정
PDF 원문은 일관되게 `Func`(대문자 F)를 사용한다. 기존 키워드(`var`, `if` 등)는 모두 소문자.

| 선택지 | 결정 |
|--------|------|
| A) `Func` (PDF 원문) | **채택** |
| B) `func` (소문자 통일) | 미채택 |

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `Token.h` | 수정 | `COMMA`, `FUNC`, `RETURN` 3개 TokenType 추가 |
| `Lexer.cpp` | 수정 | keywords 맵에 `Func`→`FUNC`, `return`→`RETURN` 추가 / `scanToken()`에 `','→COMMA` 추가 |
| `docs/tokenizing_추가요구사항분석_function.md` | 신규 | 함수 지원 토큰 요구사항 분석 문서 |

### 변경 상세

#### Token.h
```cpp
// Single-char tokens에 추가
COMMA,

// Keywords에 추가
FUNC, RETURN,
```

#### Lexer.cpp — keywords 맵
```cpp
{"Func",   TokenType::FUNC},   // 함수 선언 키워드 (PDF 원문 기준 대문자 F)
{"return", TokenType::RETURN}, // 반환문 키워드
```

#### Lexer.cpp — scanToken()
```cpp
case ',': addToken(TokenType::COMMA); break;
```

### 테스트 결과
```
[==========] Running 86 tests from 7 test suites.
[  PASSED  ] 86 tests.
```

### 현재 TDD 단계
```
[RED]      테스트 케이스 미추가 (TC-LEX-021~029 예정)
[GREEN]    기존 86개 전부 통과 확인
[REFACTOR] 미착수
```

### 다음 세션 목표
논리 연산자(`and`, `or`, `!`) 토큰 지원 추가.

---

## 세션 012 — 논리 연산자 Token/Lexer 수정 (2026-06-05)

### 커밋
```
47d8cab  [feat] Add logical operator tokenizing support (and, or, !)
```

### 사용자 프롬프트 원문
```
아니 지금은 unit test case 를 넣는 것보다 다른 동료들이 기다리고 있어서 다음
feature 를 넣어야 하는 상황이야. 논리 연산자 중에 "and", "or", "!" 는 지원하는
것으로 결정이 났어. 해당 연산자들이 tokenizing 되는지 확인해주고, 필요하다면
코드 작업 진행하자.
```

### 주목적
`Token.h`에 이미 선언된 `AND_OP`, `OR_OP`, `BANG`이 Lexer에서 실제로 처리되는지 확인하고 누락된 처리를 추가한다.

### 사전 점검 결과

| 연산자 | 기존 상태 | 문제 |
|--------|-----------|------|
| `and` | keywords 맵 미등록 | `IDENTIFIER`로 잘못 처리됨 |
| `or` | keywords 맵 미등록 | `IDENTIFIER`로 잘못 처리됨 |
| `!` | `!=`일 때만 처리 | 단독 `!` → `LexError` throw |

> `AND_OP`, `OR_OP`, `BANG` 토큰 타입은 `Token.h`에 이미 존재(팀원 선작업)했으나 Lexer 연결이 누락된 상태였음.

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `Lexer.cpp` | 수정 | keywords 맵에 `and`→`AND_OP`, `or`→`OR_OP` 추가 / `!` 단독 처리 → `BANG` |

### 변경 상세

#### Lexer.cpp — keywords 맵
```cpp
{"and", TokenType::AND_OP}, // 논리 AND 연산자
{"or",  TokenType::OR_OP},  // 논리 OR 연산자
```

#### Lexer.cpp — scanToken() `case '!'` 수정
```cpp
// Before: 단독 ! → LexError
case '!':
    if (match('=')) {
        addToken(TokenType::BANG_EQUAL);
    } else {
        throw LexError(line_, std::string("unexpected character '") + c + "'");
    }
    break;

// After: 단독 ! → BANG
case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
```

### 테스트 결과
```
[==========] Running 86 tests from 7 test suites.
[  PASSED  ] 86 tests.
```

### 현재 TDD 단계
```
[RED]      테스트 케이스 미추가 (함수/논리 연산자 관련 TC 작성 예정)
[GREEN]    기존 86개 전부 통과 확인
[REFACTOR] 미착수
```

### 다음 세션 목표
함수 및 논리 연산자 관련 Lexer 테스트 케이스(TC-LEX-021~) 추가.

---

## 세션 013 — 테스트 파일명 클래스명 일치 리팩터링 (2026-06-05)

### 커밋
```
(이번 커밋에 포함)
```

### 사용자 프롬프트 원문
```
커밋하기 전에 lexter_test.cpp 이름을 class 랑 동일하게 바꾸고 싶어
```

```
개발문서에 지금 내용 업데이트 같이 해주고, 같이 스테이징 진행해줘
```

### 주목적
파일명을 클래스명(`LexerTestFixture`)과 일치시켜 코드 탐색 일관성을 높인다.

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `test_lexer.cpp` → `LexerTestFixture.cpp` | 이름 변경 | 클래스명(`LexerTestFixture`)과 파일명 일치 |
| `CodeFab_Interpreter.vcxproj` | 수정 | `ClCompile` 참조 경로 `test_lexer.cpp` → `LexerTestFixture.cpp` |
| `CodeFab_Interpreter.vcxproj.filters` | 수정 | 필터 참조 경로 동일하게 변경 (2곳) |
| `docs/tokenizing_개발_추가요구사항.md` | 신규 | 오늘 세션 010~013 개발 히스토리 문서화 |

### 테스트 결과
```
[==========] Running 86 tests from 7 test suites.
[  PASSED  ] 86 tests.
```

### 현재 TDD 단계
```
[RED]      테스트 케이스 미추가 (함수/논리 연산자 관련 TC 작성 예정)
[GREEN]    기존 86개 전부 통과 확인
[REFACTOR] ✅ 파일명 정비 완료
```

### 다음 세션 목표
함수 및 논리 연산자 관련 Lexer 테스트 케이스(TC-LEX-021~) 추가.

---

## 세션 014 — 배열 지원 Token/Lexer 수정 (2026-06-05)

### 커밋
```
(이번 커밋에 포함)
```

### 사용자 프롬프트 원문
```
추가 기능 중에 array 지원해야 하는 것도 있어, 하나의 PR request 로 진행할
예정이라 지금 git 에서 추가 구현을 해줘. 같은 작업을 하면 돼.
```

### 주목적
Chapter 3 정적 배열 요구사항에 따라 인덱스 접근에 필요한 `[`, `]` 토큰을 추가한다.

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `Token.h` | 수정 | `LEFT_BRACKET`, `RIGHT_BRACKET` 2개 TokenType 추가 |
| `Lexer.cpp` | 수정 | `scanToken()`에 `'['→LEFT_BRACKET`, `']'→RIGHT_BRACKET` 처리 추가 |

### 변경 상세

#### Token.h
```cpp
// Single-char tokens에 추가
LEFT_BRACKET, RIGHT_BRACKET,
```

#### Lexer.cpp — scanToken()
```cpp
case '[': addToken(TokenType::LEFT_BRACKET);  break;
case ']': addToken(TokenType::RIGHT_BRACKET); break;
```

### 테스트 결과
```
[==========] Running 86 tests from 7 test suites.
[  PASSED  ] 86 tests.
```

### 현재 TDD 단계
```
[RED]      테스트 케이스 미추가 (배열 관련 TC 작성 예정)
[GREEN]    기존 86개 전부 통과 확인
[REFACTOR] 미착수
```

### 다음 세션 목표
PR 생성 후 팀원 코드 리뷰 진행.

---

*이 문서는 세션이 종료될 때마다 새 세션 항목을 추가하여 누적 관리한다.*
