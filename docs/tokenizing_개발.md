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

*이 문서는 세션이 종료될 때마다 새 세션 항목을 추가하여 누적 관리한다.*
