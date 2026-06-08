# CODE_COVERAGE_현재상태

**작성일**: 2026-06-08  
**측정 도구**: OpenCppCoverage 0.9.9.0  
**측정 대상**: `x64/Debug/CodeFab_Interpreter.exe`  
**측정 범위**: 유닛 테스트(162 TC) + TestScripts 82개 스크립트

---

## 1. 측정 방법

```
[1단계] unit 모드 → binary 커버리지 저장
[2단계] run <file> 모드 × 82개 스크립트 → 누적
[3단계] HTML 리포트 생성
```

측정 자동화 스크립트: `run_coverage.ps1`

---

## 2. 전체 커버리지 요약

| 범위 | 커버리지 |
|------|---------|
| 전체 (packages/gmock 포함) | **48%** |
| **프로젝트 핵심 파이프라인** (Debugger 제외) | **~85%** |
| **프로젝트 전체** (Debugger 포함) | **~67%** |

> `packages/` 디렉토리(gmock/gtest 라이브러리)의 미사용 라인이 전체 수치를 끌어내림.

---

## 3. 파일별 상세 커버리지

### 핵심 파이프라인 (Lexer → Parser → CheckerUnit → Interpreter)

| 파일 | 커버리지 | 측정 라인 수 |
|------|---------|------------|
| `Lexer.h` | 100% | 6 |
| `Lexer.cpp` | 100% | 97 |
| `Token.h` | 100% | 8 |
| `Parser.h` | 100% | 2 |
| `CheckerUnit/CheckerError.h` | 100% | 2 |
| `InterpreterFactory/InterpreterRunner.h` | 100% | 1 |
| `Parser.cpp` | 99% | 202 |
| `Expr.h` | 98% | 54 |
| `CheckerUnit/CheckerUnit.cpp` | 96% | 167 |
| `Callable.cpp` | 89% | 19 |
| `CheckerUnit/CheckEvalOperation.cpp` | 88% | 69 |
| `Callable.h` | 87% | 8 |
| `InterpreterFactory/FileRunner.cpp` | 84% | 33 |
| `Stmt.h` | 84% | 51 |
| `Interpreter.cpp` | 83% | 255 |
| `Environment.h` | 80% | 5 |
| `Environment.cpp` | 70% | 27 |
| `Interpreter.h` | 66% | 3 |
| `main.cpp` | 65% | 23 |
| `InterpreterFactory/InterpreterFactory.cpp` | 57% | 7 |

### Debugger 모듈 (대화형 — 자동 측정 불가)

| 파일 | 커버리지 | 이유 |
|------|---------|------|
| `Debugger/Debugger.cpp` | 0% | `debug <file>` 모드 미실행 |
| `Debugger/DebugCommandParser.cpp` | 0% | 동일 |
| `Debugger/Commands/FlowCommands.cpp` | 0% | 동일 |
| `Debugger/Commands/BreakCommands.cpp` | 0% | 동일 |
| `Debugger/Commands/WatchCommands.cpp` | 0% | 동일 |
| `InterpreterFactory/ReplRunner.cpp` | 0% | REPL 모드(대화형) 미실행 |
| `InterpreterFactory/DebugRunner.cpp` | 0% | 동일 |

---

## 4. 커버리지 미달 원인 분석

### 자동화 가능한 미커버 경로

| 파일 | 미커버 경로 | 입력 조건 |
|------|-----------|---------|
| `Interpreter.cpp` | `visitIndexSetExpr` — 비배열에 할당 | `var x=5; x[0]=1;` |
| `Interpreter.cpp` | `visitIndexSetExpr` — 비숫자 인덱스 | `var a=Array(2); a[true]=9;` |
| `Interpreter.cpp` | `visitIndexSetExpr` — 범위 초과 할당 | `var a=Array(2); a[5]=9;` |
| `Environment.cpp` | `tryGetValue()` — enclosing 체인 탐색 | 중첩 스코프 tryGetValue |
| `Environment.cpp` | `tryGetValue()` — nullopt 반환 | 미정의 변수 tryGetValue |
| `InterpreterFactory.cpp` | `default` throw — 잘못된 RunMode | `RunMode(999)` |

### 자동화 불가 경로

| 파일 | 미커버 경로 | 이유 |
|------|-----------|------|
| `main.cpp` | REPL 분기 (argc==1) | `std::cin` 대기, 대화형 |
| `main.cpp` | debug 분기 | Debugger 대화형 명령 대기 |
| `Debugger/*` | 전체 | 대화형 세션 필요 |
| `ReplRunner.cpp` | 전체 | 대화형 세션 필요 |

### 도달 불가 Dead Code (측정 의미 없음)

| 파일 | 경로 | 이유 |
|------|------|------|
| `Interpreter.cpp` | `visitUnaryExpr` default | 현재 MINUS/BANG만 존재 |
| `Interpreter.cpp` | `visitBinaryExpr` default | 모든 이항 연산자 처리됨 |
| `CheckEvalOperation.cpp` | `evaluateArithmeticOp` default | 내부 helper, 정해진 타입만 전달 |
| `CheckEvalOperation.cpp` | `evaluateComparisonOp` default | 동일 |
| `CheckEvalOperation.cpp` | `evaluateEqualityOp` — ArrayPtr/CallablePtr | 정적 분석 대상 아님 |

---

## 5. 커버리지 향상을 위한 추가 TC 목록

### 추가된 TC (이 문서 작성 시점)

| TC | 종류 | 목적 |
|----|------|------|
| `errors/err_18_array_set_non_array.txt` | 스크립트 | `visitIndexSetExpr` 비배열 에러 |
| `errors/err_19_array_set_nonnumber_index.txt` | 스크립트 | `visitIndexSetExpr` 비숫자 인덱스 에러 |
| `errors/err_20_array_set_out_of_bounds.txt` | 스크립트 | `visitIndexSetExpr` 범위 초과 에러 |
| `EnvironmentTest` — `TryGetValueFindsInEnclosing` | 유닛 TC | `tryGetValue` enclosing 체인 |
| `EnvironmentTest` — `TryGetValueReturnsNulloptWhenMissing` | 유닛 TC | `tryGetValue` nullopt 반환 |
| `InterpreterFactoryTest` — `InvalidRunModeThrows` | 유닛 TC | 잘못된 RunMode 예외 |
| `InterpreterFactoryTest` — `FileRunModeCreatesRunner` | 유닛 TC | FILE_RUN 생성 정상 확인 |

### 예상 커버리지 향상

| 파일 | 현재 | 예상 |
|------|------|------|
| `Interpreter.cpp` | 83% | ~92% |
| `Environment.cpp` | 70% | ~85% |
| `InterpreterFactory.cpp` | 57% | ~72% |
| **프로젝트 핵심 전체** | **~85%** | **~90%** |

---

## 6. HTML 리포트

```
coverage_report/index.html
```

`run_coverage.ps1` 재실행으로 최신 수치 갱신 가능.
