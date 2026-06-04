# MainPipelineTest 개발 문서

> **규칙**: 이 문서는 `feature/main_divided_gmock` 브랜치의 MainPipelineTest 개발 히스토리를 기록한다.  
> 새로운 작업이 완료될 때마다 **세션** 단위로 추가하며, 사용자 프롬프트 원문을 반드시 포함한다.

---

## 목차

- [프로젝트 개요](#프로젝트-개요)
- [세션 001 — Debug/Release 빌드 분기 (2026-06-04)](#세션-001--debugrelease-빌드-분기-2026-06-04)
- [세션 002 — 브랜치 생성 및 CheckerUnit 크래시 수정 (2026-06-04)](#세션-002--브랜치-생성-및-checkerunit-크래시-수정-2026-06-04)
- [세션 003 — MainPipelineTest 작성 (2026-06-04)](#세션-003--mainpipelinetest-작성-2026-06-04)

---

## 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 과제 | C++17 기반 커스텀 언어 인터프리터 구현 (CodeFab 교육 과정) |
| 브랜치 | `feature/main_divided_gmock` |
| 빌드 환경 | MSVC 19.51+, C++17, Google Mock 1.11.0 (NuGet) |
| 처리 파이프라인 | `Lexer → Parser → Interpreter` (Environment 포함) |

---

## 세션 001 — Debug/Release 빌드 분기 (2026-06-04)

### 커밋
```
(미커밋 — 추후 진행)
```

### 사용자 프롬프트 원문
```
이제부터 새로운 작업을 시작하려고 해. @CodeFab_Interpreter/main.cpp 에는 현재
gmock test 만 가능하도록 되어 있는데, 이를 debug build 할 때는 gmock 이 수행이
되도록 하고, release build 일 때는 user input 을 받을 수 있도록 수정하려고 해.
user input 을 file 입력으로 할 것인지, 직접 입력하는 방식으로 할 것인지는
다음 작업에 할 예정이고, 우선 당장은 debug build 일 때, gmock test 를 진행하되,
지금까지 추가된 항목들을 mocking 을 통해서 한번에 동작하게 하려고 하고 있어.
말한대로 수정해 줄래? 지금부터 작업하는 부분은 새로운 개발 문서에 히스토리와
내가 쓴 prompt 를 모두 같이 정리할 수 있도록 해줘.
```

### 주목적
Debug/Release 빌드 목적을 명확히 분리한다.
- **Debug**: Google Mock 테스트 러너 — 개발 중 단위 테스트 수행
- **Release**: 사용자 입력 모드 — 실제 인터프리터 실행 (추후 구현)

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `CodeFab_Interpreter/main.cpp` | 수정 | `#ifdef _DEBUG` 분기로 Debug/Release 동작 분리 |
| `docs/main_MainPipelineTest_개발.md` | 신규 | 이 문서 |

### 변경 내용

```cpp
// Before
#include "gmock/gmock.h"

int main() {
    testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}

// After
#ifdef _DEBUG
#include "gmock/gmock.h"

int main() {
    testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}

#else

int main() {
    // Release: 사용자 입력 모드 (추후 구현 예정)
    return 0;
}

#endif
```

### 설계 결정 사항

| 항목 | 결정 | 이유 |
|------|------|------|
| 분기 매크로 | `_DEBUG` | MSVC Debug 구성에서 자동 정의되는 표준 매크로 |
| gmock `#include` 위치 | `#ifdef _DEBUG` 블록 내부 | Release 빌드에서 불필요한 헤더 포함 방지 |
| Release `main()` 본문 | stub (return 0) | 사용자 입력 방식(파일/직접 입력)은 다음 세션에서 결정 |

---

## 세션 002 — 브랜치 생성 및 CheckerUnit 크래시 수정 (2026-06-04)

### 커밋
```
5668c6b  [fix] CheckerUnit 테스트 doChecker 호출 전 스코프 초기화 누락 수정
```

### 사용자 프롬프트 원문
```
우선 새로운 branch 부터 생성하자. 새로운 branch 이름은 feature/main_divided_gmock
위 부분을 수정하지 말고 unit test 에서 한번이라도 push 한 다음에 위 함수를 호출 할 수 있도록 수정해줄래?
checkerunit 만 commit 진행해줘. 제목에 [fix] prefix 붙여서 진행해줘.
```

### 주목적
- `feature/main_divided_gmock` 브랜치를 신규 생성한다.
- 다른 팀원이 구현한 `CheckerUnit`의 크래시를 수정하여 전체 테스트가 실행될 수 있도록 한다.

### 원인 분석
`CheckerUnit::visitVarStmt()`에서 `check_values_in_scopes_vector.back()`을 호출하는데,
`doChecker()` 진입 시 스코프가 한 번도 push되지 않아 빈 벡터에 접근 → 크래시 발생.

```
Assertion failed: back() called on empty vector (vector(1994))
```

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `CheckerUnit/CheckerUnit.h` | 수정 | `beginScope()` / `endScope()` public 메서드 추가 |
| `CheckerUnit/UnitTest/CheckerUnitTest.cpp` | 수정 | `SetUp()`에서 `beginScope()`, `TearDown()`에서 `endScope()` 호출 |

### 설계 결정 사항

| 항목 | 결정 | 이유 |
|------|------|------|
| 수정 위치 | `CheckerUnit.h` + 테스트 | `doChecker()` 본문은 팀원 코드이므로 수정하지 않음 |
| `beginScope()` 구현 | inline (`push_back({})`) | 헤더에 직접 구현하여 `.cpp` 수정 최소화 |

### 테스트 결과
```
[==========] Running 58 tests from 6 test suites.
[  PASSED  ] 58 tests.
```

---

## 세션 003 — MainPipelineTest 작성 (2026-06-04)

### 커밋
```
(미커밋 — 추후 진행)
```

### 사용자 프롬프트 원문
```
이제 main 에 대한 unit test case 를 추가하는 작업을 진행할 거야. 우선 지금까지
협업된 코드들을 분석하고, interpreter 기반 TC 자체를 기반으로 동작하는 unit test
case 를 새로운 파일에다가 작성해줄래?
PipelineTestFixture 라는 이름을 MainPipelineTestFixture 으로 변경해주고,
각 test 함수 시작할 때, 해당 unit test 가 어느 pipeline 들을 체크하는지 주석을
통해서 알기 쉽게 정리해주고, 만약 다른 객체를 불러줄 경우엔 왜 불러주는지에
대한 정보도 같이 남겨줘. 파일 이름도 변경되는 객체 이름에 맞춰 바꿔줘
```

### 주목적
소스 문자열을 실제 `Lexer → Parser → Interpreter` 파이프라인에 통과시켜 통합 동작을 검증하는 테스트를 작성한다.
기존 `InterpreterTest` / `InterpreterStmtTest`는 `TestHelpers`로 AST를 직접 조립하므로 Lexer/Parser가 우회된다.

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `CodeFab_Interpreter/MainPipelineTest.cpp` | 신규 | 전체 파이프라인 통합 테스트 14개 |
| `CodeFab_Interpreter/CodeFab_Interpreter.vcxproj` | 수정 | `MainPipelineTest.cpp` 빌드 등록 |

### 테스트 케이스 목록 (TC-PIPE-001 ~ 011)

| ID | 검증 내용 | 체크 파이프라인 | 예상 결과 |
|----|-----------|----------------|-----------|
| TC-PIPE-001 | 빈 입력 | Lexer → Parser → Interpreter | 오류 없음 |
| TC-PIPE-002 | 공백·탭·개행만 | Lexer → Parser → Interpreter | 오류 없음 |
| TC-PIPE-003 | 주석만 있는 소스 | Lexer → Parser → Interpreter | 오류 없음 |
| TC-PIPE-004 | 단일 var 선언 | Lexer → Parser → Interpreter → Environment | 오류 없음 |
| TC-PIPE-005 | 복수 var 선언 | Lexer → Parser → Interpreter → Environment | 오류 없음 |
| TC-PIPE-006 | var 선언 후 식 참조 | Lexer → Parser → Interpreter → Environment | 오류 없음 |
| TC-PIPE-007 | 변수를 다른 변수로 초기화 | Lexer → Parser → Interpreter → Environment | 오류 없음 |
| TC-PIPE-008 | 미정의 변수 참조 | Lexer → Parser → Interpreter → Environment | `RuntimeError` |
| TC-PIPE-009 | 미정의 변수를 초기화에 사용 | Lexer → Parser → Interpreter → Environment | `RuntimeError` |
| TC-PIPE-010 | 주석 + var 선언 혼재 | Lexer → Parser → Interpreter → Environment | 오류 없음 |
| TC-PIPE-011 | 소수 리터럴 var 선언 | Lexer → Parser → Interpreter → Environment | 오류 없음 |

### Parser 현재 지원 범위
```
- var 선언 : var name = NUMBER;
- 식 문     : IDENTIFIER;
→ print / if / else / for / 블록 / 연산자는 Parser 확장 후 TC 추가 예정
```

### 테스트 결과
```
[==========] Running 69 tests from 7 test suites.
[  PASSED  ] 69 tests.
```

---

*이 문서는 세션이 종료될 때마다 새 세션 항목을 추가하여 누적 관리한다.*
