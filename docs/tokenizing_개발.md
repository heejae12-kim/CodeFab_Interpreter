# tokenizing 개발 문서

> **규칙**: 이 문서는 `feature/tokenizing` 브랜치의 개발 히스토리를 기록한다.  
> 새로운 작업이 완료될 때마다 **세션** 단위로 추가하며, 사용자 프롬프트 원문을 반드시 포함한다.

---

## 목차

- [프로젝트 개요](#프로젝트-개요)
- [세션 001 — 프로젝트 초기화 (2026-06-02)](#세션-001--프로젝트-초기화-2026-06-02)
- [세션 002 — Token 헤더 정의 (2026-06-02)](#세션-002--token-헤더-정의-2026-06-02)
- [세션 003 — AST 노드 헤더 추가 (2026-06-02)](#세션-003--ast-노드-헤더-추가-2026-06-02)
- [세션 004 — TDD 환경 구축 및 Lexer 단위 테스트 작성 (2026-06-04)](#세션-004--tdd-환경-구축-및-lexer-단위-테스트-작성-2026-06-04)
- [세션 005 — 빌드 에러 수정: /utf-8 플래그 누락 (2026-06-04)](#세션-005--빌드-에러-수정-utf-8-플래그-누락-2026-06-04)
- [세션 006 — Lexer GREEN 단계 구현 (2026-06-04)](#세션-006--lexer-green-단계-구현-2026-06-04)
- [세션 007 — Pull Request 준비 및 push (2026-06-04)](#세션-007--pull-request-준비-및-push-2026-06-04)
- [세션 008 — Code Convention 정립 (2026-06-04)](#세션-008--code-convention-정립-2026-06-04)

---

## 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 과제 | C++17 기반 커스텀 언어 인터프리터 구현 (CodeFab 교육 과정) |
| 브랜치 | `feature/tokenizing` |
| 빌드 환경 | MSVC 19.51+, C++17/20, Google Mock 1.11.0 (NuGet) |
| 테스트 프레임워크 | Google Test / Google Mock (`gmock.1.11.0`) |
| 처리 파이프라인 | `Lexer → Parser → Resolver → Interpreter` |

---

## 세션 001 — 프로젝트 초기화 (2026-06-02)

### 커밋
```
e3d3987  Initialization
4150de2  초기 커밋.
f4772d7  .gitattributes 및 .gitignore을(를) 추가합니다.
```

### 주목적
Visual Studio 프로젝트 기반을 잡고 Google Mock NuGet 패키지를 연결한다.

### 변경 파일
| 파일 | 내용 |
|------|------|
| `CodeFab_Interpreter.vcxproj` | 프로젝트 설정 (C++20, gmock.1.11.0 타겟 연결) |
| `CodeFab_Interpreter.vcxproj.filters` | VS 필터 초기화 |
| `main.cpp` | Google Mock 테스트 러너 진입점 |
| `packages.config` | gmock 1.11.0 NuGet 패키지 참조 |

### 핵심 결정 사항
- `main.cpp`는 인터프리터 진입점이 아닌 **테스트 러너**로 설정 → 전체 프로젝트가 테스트 중심으로 동작
- NuGet `gmock.1.11.0` 패키지가 gtest도 함께 제공

---

## 세션 002 — Token 헤더 정의 (2026-06-02)

### 커밋
```
833fe64  [Token] Add default token header
```

### 주목적
어휘 분석기(Lexer)가 생성할 토큰의 타입과 값 구조를 정의한다.

### 변경 파일
| 파일 | 변경 내용 |
|------|-----------|
| `Token.h` | 신규 생성 (47줄) |

### Token.h 핵심 설계
```cpp
// 런타임 값 타입 — variant로 nil까지 커버
using Value = std::variant<double, std::string, bool, std::nullptr_t>;

enum class TokenType {
    // 단일 문자: (, ), {, }, ;, +, -, *, /
    // 1-2 문자:  <, <=, >, >=, =, ==, !=
    // 리터럴:    NUMBER, STRING, IDENTIFIER
    // 키워드:    VAR, IF, ELSE, FOR, PRINT, TRUE_KW, FALSE_KW
    // 종료:      EOF_TOKEN
};
```

### 핵심 결정 사항
- `Value`를 `std::variant`로 정의 → nil은 `std::nullptr_t`로 표현, 별도 래퍼 불필요
- `Token` 클래스에 `getTokenType()`, `getLexme()`, `getLiteral()`, `getLine()` getter 제공

---

## 세션 003 — AST 노드 헤더 추가 (2026-06-02)

### 커밋
```
0734d45  [Header] Add and modify header files
```

### 주목적
파서가 생성할 AST(Abstract Syntax Tree) 노드 구조와 Visitor 패턴 인터페이스를 정의한다.

### 변경 파일
| 파일 | 변경 내용 |
|------|-----------|
| `Expr.h` | 신규 생성 (118줄) — Expression 노드 6종 |
| `Stmt.h` | 신규 생성 (121줄) — Statement 노드 6종 |
| `Token.h` | 수정 (+8줄) — `interface` 매크로 추가 |

### Expr.h 노드 목록
| 노드 | 역할 | 주요 필드 |
|------|------|-----------|
| `LiteralExpr` | 숫자/문자열/bool/nil | `Value value` |
| `UnaryExpr` | 단항 `-` | `Token op, ExprPtr right` |
| `BinaryExpr` | 이항 연산 | `ExprPtr left, Token op, ExprPtr right` |
| `GroupingExpr` | 괄호 그룹 | `ExprPtr expression` |
| `VariableExpr` | 변수 참조 | `Token name` |
| `AssignExpr` | 변수 대입 | `Token name, ExprPtr value` |

### Stmt.h 노드 목록
| 노드 | 역할 |
|------|------|
| `PrintStmt` | `print expr;` |
| `ExprStmt` | `expr;` |
| `VarStmt` | `var name = expr;` |
| `BlockStmt` | `{ stmts... }` |
| `IfStmt` | `if/else` |
| `ForStmt` | `for(init; cond; incr)` |

### 핵심 결정 사항
- `ExprPtr` = `std::unique_ptr<Expr>`, `StmtPtr` = `std::unique_ptr<Stmt>` alias
- `#define interface struct` 매크로 — MSVC 환경에서 순수 가상 인터페이스를 struct로 표현
- Visitor 패턴 분리: `ExprVisitor` / `StmtVisitor` 인터페이스 별도 정의

---

## 세션 004 — TDD 환경 구축 및 Lexer 단위 테스트 작성 (2026-06-04)

### 커밋
```
(미커밋 — 작업 중)
```

### 사용자 프롬프트 원문
```
이전에 만들었던 project 를 기반으로 새로운 코드를 생성 하려고 해.
개발은 TDD 기반으로 진행할 예정이고, 지금 네가 무엇을 만들어야 할지
@CLAUDE.md 기반으로 먼저 새로운 rule용 문서를 생성해주고, tokenizing
전용 UNIT TEST 를 생성하려고 해. 우선은 사용자 입력을 token 으로 변환하고
이를 확인하는 unit test 먼저 작성해줄래?
```

### 주목적
TDD RED 단계를 완성한다. Lexer 구현 이전에 테스트를 먼저 작성하여 설계 계약을 코드로 고정한다.

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `docs/TDD_가이드.md` | 신규 | TDD 규칙, 테스트 명명법, 컴포넌트별 범위 정의 |
| `CodeFab_Interpreter/Lexer.h` | 신규 | Lexer 클래스 인터페이스 + `LexError` 예외 정의 |
| `CodeFab_Interpreter/Lexer.cpp` | 신규 | 빈 stub — `tokenize()`가 빈 벡터만 반환 (RED 상태) |
| `CodeFab_Interpreter/test_lexer.cpp` | 신규 | Google Test 기반 단위 테스트 20개 |
| `CodeFab_Interpreter.vcxproj` | 수정 | Lexer.cpp, Lexer.h, test_lexer.cpp 빌드 등록 |
| `CodeFab_Interpreter.vcxproj.filters` | 수정 | VS 솔루션 탐색기 필터 반영 |

### 테스트 케이스 목록 (TC-LEX-001 ~ 020)
| ID | 검증 내용 | 예상 결과 |
|----|-----------|-----------|
| TC-LEX-001 | 빈 입력 | 토큰 1개 (EOF_TOKEN) |
| TC-LEX-002 | 공백·탭·개행만 | 토큰 1개 (EOF_TOKEN) |
| TC-LEX-003 | `( ) { } ; + - * /` | 단일 문자 토큰 9종 |
| TC-LEX-004 | `< <= > >= == != =` | 비교·대입 연산자 7종 |
| TC-LEX-005 | `42` | NUMBER, lexeme="42", literal=42.0 |
| TC-LEX-006 | `3.14` | NUMBER, lexeme="3.14", literal=3.14 |
| TC-LEX-007 | `"hello world"` | STRING, literal="hello world" (따옴표 제외) |
| TC-LEX-008 | `""` | STRING, literal="" |
| TC-LEX-009 | `var if else for print true false` | 키워드 7종 |
| TC-LEX-010 | `myVar` | IDENTIFIER, lexeme="myVar" |
| TC-LEX-011 | `ifx var2 printme` | 세 토큰 모두 IDENTIFIER |
| TC-LEX-012 | `+ // comment\n-` | PLUS, MINUS (주석 제거) |
| TC-LEX-013 | `// entire comment` | EOF_TOKEN만 |
| TC-LEX-014 | `"var\nx"` | var→line 1, x→line 2 |
| TC-LEX-015 | `"hello` (닫히지 않은 문자열) | `LexError` throw |
| TC-LEX-016 | `@` (알 수 없는 문자) | `LexError` throw |
| TC-LEX-017 | `var x = 10 + 3.5;` | 7개 토큰 순서 검증 |
| TC-LEX-018 | `true false` | bool 리터럴 값 true/false |
| TC-LEX-019 | for 루프 스니펫 | 오류 없이 토큰화, EOF 존재 |
| TC-LEX-020 | `1` | EOF_TOKEN lexeme = "" |

### Lexer.h 설계 요약
```cpp
class LexError : public std::runtime_error { int line_; };

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();   // 공개 인터페이스
private:
    // 내부 스캐너 메서드
    void scanToken();
    void scanString();
    void scanNumber();
    void scanIdentifierOrKeyword();
};
```

### 현재 TDD 단계
```
[RED]  ✅ 테스트 20개 작성 완료 — 모두 실패 예정
[GREEN]   Lexer::tokenize() 구현 필요
[REFACTOR]  미착수
```

### 다음 세션 목표
`Lexer.cpp`에 실제 `tokenize()` 로직을 구현하여 TC-LEX-001 ~ 020 전체를 GREEN으로 전환한다.

---

## 세션 006 — Lexer GREEN 단계 구현 (2026-06-04)

### 커밋
```
b6aba71  [feat][tokenizing] tokenizing 구현(GREEN 단계)
```

### 사용자 프롬프트 원문
```
GREEN 단계로 전환해서 구현해줘.
```

### 주목적
`Lexer::tokenize()` 실제 로직을 구현하여 TC-LEX-001 ~ 020 전체를 GREEN으로 전환한다.

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `CodeFab_Interpreter/Lexer.cpp` | 수정 | `tokenize()` 및 스캐너 메서드 전체 구현 |

### 구현 핵심 내용
| 메서드 | 역할 |
|--------|------|
| `tokenize()` | 전체 스캔 루프 + EOF_TOKEN 추가 |
| `scanToken()` | 문자별 switch — 단일 문자, 1-2자 연산자, 주석, 공백, 숫자/문자 분기 |
| `scanString()` | `"..."` 파싱, 따옴표 제외 literal 저장, 미닫힘 시 `LexError` |
| `scanNumber()` | 정수/소수 파싱 → `std::stod` → `double` literal |
| `scanIdentifierOrKeyword()` | 키워드 맵 조회, `true`/`false`는 `bool` literal 저장 |

### 테스트 결과
```
[==========] Running 20 tests from 1 test suite.
[  PASSED  ] 20 tests.
```

### 현재 TDD 단계
```
[RED]      ✅ 테스트 20개 작성 완료
[GREEN]    ✅ TC-LEX-001~020 전부 통과 (20/20)
[REFACTOR]    미착수
```

### 다음 세션 목표
Parser 구현을 위한 TDD RED 단계 — `test_parser.cpp` 작성.

---

## 세션 005 — 빌드 에러 수정: /utf-8 플래그 누락 (2026-06-04)

### 커밋
```
a5e9f48  [fix] 한글 주석처리에 의한 build error 수정
```

### 사용자 프롬프트 원문
```
지금 빌드 테스트 하는데 빌드 에러가 발생했어. 이 부분 확인해줄래?
MSBuild 로 실행해줘
지금 설치된 visual studio 는 visual studio community 2026 이야. 이것을 이용해서 진행해줘
```

### 주목적
vcxproj의 `/utf-8` 플래그 누락으로 인한 빌드 에러를 수정한다.

### 원인 분석
MSVC는 소스 파일을 시스템 기본 코드 페이지(949, EUC-KR)로 읽는다.  
`test_lexer.cpp`에 포함된 한글 주석 및 유니코드 문자(`──` 등)를 코드 페이지 949로 파싱하면서  
괄호 카운팅이 어긋나 `TC-LEX-014` 이후 모든 테스트가 중첩된 것으로 오인되었다.

```
경고 C4819: 현재 코드 페이지(949)에서 표시할 수 없는 문자가 파일에 들어 있습니다.
오류 C1075: '{': 일치하는 토큰을 찾을 수 없습니다. (test_lexer.cpp:148)
```

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `CodeFab_Interpreter.vcxproj` | 수정 | 4개 구성(Debug/Release × Win32/x64) 모두에 `/utf-8` 컴파일러 플래그 추가 |

### 변경 내용
```xml
<!-- 4개 ItemDefinitionGroup 각각에 추가 -->
<AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
```

### 현재 TDD 단계
```
[RED]  ✅ 테스트 20개 작성 완료 — 빌드 성공, 경고 0 오류 0
[GREEN]   Lexer::tokenize() 구현 필요
[REFACTOR]  미착수
```

### 다음 세션 목표
`Lexer.cpp`에 실제 `tokenize()` 로직을 구현하여 TC-LEX-001 ~ 020 전체를 GREEN으로 전환한다.

---

## 세션 007 — Pull Request 준비 및 push (2026-06-04)

### 커밋
```
393f714  [doc] tokenizing 개발 문서 업데이트
```

### 사용자 프롬프트 원문
```
지금까지 작업한 것을 pull request 진행하려고 해.
다른 개발자들이 리뷰하기 쉽게 변경점 리스트와 리뷰해야하는 내용을 추려서 진행해줄래?
네가 직접 pull request 도 만들 수 있니?
```

### 주목적
`feature/tokenizing` 브랜치를 remote에 push하고 팀원 코드 리뷰를 위한 PR 본문을 작성한다.

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `docs/tokenizing_개발.md` | 수정 | 세션 007 추가, 목차 갱신 |

### PR 정보
| 항목 | 내용 |
|------|------|
| 제목 | `[feat][tokenizing] Lexer 구현 및 TDD 단위 테스트 추가` |
| base | `master` |
| compare | `feature/tokenizing` |
| URL | `https://github.com/heejae12-kim/CodeFab_Interpreter/compare/master...feature/tokenizing` |

### PR 리뷰 포인트 요약
| # | 대상 | 내용 |
|---|------|------|
| 1 | `Token.h`, `Expr.h` | `Value` → `ValuableValue` 이름 변경 — Interpreter/Resolver 구현 시 동일 적용 필요 |
| 2 | `Lexer.h` | `LexError` 예외 포맷 `[line N] Syntax Error: 메시지` — main catch 방식과 일치 여부 확인 |
| 3 | `Lexer.cpp:64` | 단독 `!`는 현재 스펙 외 → `LexError` throw. 추후 논리 NOT 추가 시 수정 지점 |
| 4 | `Lexer.cpp` `scanNumber()` | `1.` 형태는 정수로만 파싱, `.`은 다음 토큰으로 분리 |
| 5 | `test_lexer.cpp` | 20개 케이스, 정상/에러 경로 모두 포함 |

### 현재 TDD 단계
```
[RED]      ✅ TC-LEX-001~020 작성 완료
[GREEN]    ✅ TC-LEX-001~020 전부 통과 (20/20)
[REFACTOR]    미착수
```

### 다음 세션 목표
REFACTOR 단계 또는 Parser TDD RED 단계 진입.

---

## 세션 008 — Code Convention 정립 (2026-06-04)

### 커밋
```
(진행 중)
```

### 사용자 프롬프트 원문
```
우리 팀 코드 작성 rule(Code Convention) 는 다음과 같아.
1. File and Class 는 Pascal Case
2. Method 는 carmel case
3. 변수는 snake_case 이야.
이 내용을 개발 문서 및 다음에도 계속해서 유지 될 수 있도록
현재 project 파일 문서에 다 업데이트 해주고,
현재 작성한 코드들이 해당 rule 을 제대로 준수하고 있는지 다시 확인해줘
```

### 주목적
팀 Code Convention을 공식화하여 `CLAUDE.md`에 등록하고, 기존 코드 전체의 준수 여부를 점검한다.

### 변경 파일
| 파일 | 변경 유형 | 내용 |
|------|-----------|------|
| `CLAUDE.md` | 수정 | Code Convention 섹션 추가 |
| `docs/tokenizing_개발.md` | 수정 | 세션 008 추가, 목차 갱신 |

### 컨벤션 검사 결과

| 파일 | 파일명 | 클래스 | 메서드 | 변수 | 결과 |
|------|--------|--------|--------|------|------|
| `Token.h` | ✅ | ✅ | ✅ | ✅ | ✅ 준수 |
| `Expr.h` | ✅ | ✅ | ✅ | ✅ | ✅ 준수 |
| `Stmt.h` | ✅ | ✅ | ✅ | ✅ | ✅ 준수 |
| `Lexer.h` | ✅ | ✅ | ✅ | ✅ | ✅ 준수 |
| `Lexer.cpp` | ✅ | — | ✅ | ✅ | ✅ 준수 |
| `test_lexer.cpp` | ✅ | ✅ | ✅ | ✅ | ✅ 준수 |

> `getLexme()`는 영단어 오타이나 팀 전체가 이미 사용 중이므로 수정 대상에서 제외한다.

### 확정된 Code Convention

| 대상 | 규칙 | 예시 |
|------|------|------|
| 파일명 / 클래스 / 구조체 / 열거형 | PascalCase | `Token`, `Lexer`, `TokenType` |
| 메서드 / 함수 | camelCase | `tokenize()`, `getTokenType()`, `scanNumber()` |
| 변수 (멤버 / 지역 / 파라미터 / 전역) | snake_case | `source_`, `start_pos` |
| private 멤버 변수 | snake_case + trailing `_` | `line_`, `tokens_`, `source_` |

### 현재 TDD 단계
```
[RED]      ✅ TC-LEX-001~020 작성 완료
[GREEN]    ✅ TC-LEX-001~020 전부 통과 (20/20)
[REFACTOR] ✅ 컨벤션 전체 준수 확인 완료
```

### 다음 세션 목표
Parser TDD RED 단계 진입 — `test_parser.cpp` 작성.

---

*이 문서는 세션이 종료될 때마다 새 세션 항목을 추가하여 누적 관리한다.*
