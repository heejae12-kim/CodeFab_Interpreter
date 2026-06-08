# CODE_COVERAGE_개선

**작성일**: 2026-06-09  
**측정 도구**: OpenCppCoverage 0.9.9.0  
**측정 범위**: `x64/Debug/CodeFab_Interpreter.exe`  
**측정 방법**: 유닛 테스트(`unit`) + 전체 TestScripts(`run <file>`) 누적 합산  
**자동화 스크립트**: `run_coverage.ps1`

---

## 1. 개요 — 작업 전/후 요약

| 항목 | Before | After | 변화 |
|------|--------|-------|------|
| 유닛 테스트 TC 수 | 162 | **166** | +4 |
| TestScripts 파일 수 | 82 | **86** | +4 |
| 테스트 스위트 수 | 9 | **10** | +1 (InterpreterFactoryTest 신규) |
| 핵심 파이프라인 커버리지 | ~85% | **~88%** | +3% |
| 전체 프로젝트 커버리지 (Debugger 포함) | ~67% | **~69%** | +2% |

---

## 2. 작업 내역 (Before → After)

### 2-1. 신규 에러 스크립트 3개 추가

`visitIndexSetExpr`의 에러 경로가 GET 계열(err_15, err_16)은 기존에 커버되어 있었으나 **SET 계열은 전혀 커버되지 않은 상태**였음.

| 파일 | 검증 경로 | 기대 출력 |
|------|----------|---------|
| `TestScripts/errors/err_18_array_set_non_array.txt` | `visitIndexSetExpr` — 비배열에 인덱스 할당 | `[Runtime Error] Only arrays support index access.` |
| `TestScripts/errors/err_19_array_set_nonnumber_index.txt` | `visitIndexSetExpr` — 비숫자 인덱스로 할당 | `[Runtime Error] Array index must be a number.` |
| `TestScripts/errors/err_20_array_set_out_of_bounds.txt` | `visitIndexSetExpr` — 범위 초과 할당 | `[Runtime Error] Array index out of range.` |

**err_18 예시**
```
var x = 5;
x[0] = 99;    // [Runtime Error] Only arrays support index access.
```

**err_19 예시**
```
var a = Array(3);
a[true] = 99; // [Runtime Error] Array index must be a number.
```

**err_20 예시**
```
var a = Array(2);
a[5] = 99;    // [Runtime Error] Array index out of range.
```

---

### 2-2. `EnvironmentTest.cpp` 유닛 TC 2개 추가

`tryGetValue()`의 enclosing 스코프 체인 탐색 경로가 미커버 상태였음.  
기존 `GetWalksUpToEnclosingScope` / `AssignWalksUpToEnclosingScope`는 `get()` / `assign()`만 검증하며, `tryGetValue()`는 별도 TC가 없었음.

| TC 이름 | 검증 내용 |
|--------|---------|
| `TryGetValueFindsInEnclosing` | 내부 스코프에서 외부 스코프 변수를 `tryGetValue`로 탐색 가능 |
| `TryGetValueReturnsNulloptWhenMissing` | 존재하지 않는 변수에 대해 `std::nullopt` 반환 |

```cpp
TEST_F(EnvironmentTest, TryGetValueFindsInEnclosing) {
    global->define("x", 42.0);
    auto inner = std::make_shared<Environment>(global);
    auto result = inner->tryGetValue("x");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<double>(*result), 42.0);
}

TEST_F(EnvironmentTest, TryGetValueReturnsNulloptWhenMissing) {
    auto result = local.tryGetValue("notExist");
    EXPECT_FALSE(result.has_value());
}
```

---

### 2-3. `InterpreterFactoryTest.cpp` 신규 파일 추가

`InterpreterFactory::create()`의 `default` throw 경로가 미커버. 테스트 파일 자체가 없었음.  
`.vcxproj`에 `<ClCompile Include="InterpreterFactoryTest.cpp" />` 등록까지 완료.

| TC 이름 | 검증 내용 |
|--------|---------|
| `InvalidRunModeThrows` | 범위 밖 `RunMode` 값 전달 시 `std::invalid_argument` 예외 발생 |
| `FileRunModeCreatesRunner` | `RunMode::FILE_RUN`으로 정상 생성 확인 |

```cpp
TEST(InterpreterFactoryTest, InvalidRunModeThrows) {
    EXPECT_THROW(
        InterpreterFactory::create(static_cast<RunMode>(999)),
        std::invalid_argument
    );
}

TEST(InterpreterFactoryTest, FileRunModeCreatesRunner) {
    EXPECT_NO_THROW(
        InterpreterFactory::create(RunMode::FILE_RUN, "dummy.txt")
    );
}
```

---

### 2-4. `TestScripts/test_50_Pass_for_block_assign.txt` 추가 (핫픽스 검증용)

커버리지 향상 목적보다는 **`for` 루프 내부 블록 변수 할당 핫픽스** 검증을 위해 추가된 스크립트.  
`Interpreter.cpp`의 for 루프 + 블록 내부 assign 경로를 추가로 커버한다.

| 항목 | 내용 |
|------|------|
| 파일명 | `TestScripts/test_50_Pass_for_block_assign.txt` |
| 기대 출력 | `0` / `1` / `3` / `3` |
| 커버 경로 | `Interpreter::visitForStmt` + `visitBlockStmt` + `visitAssignExpr` |

> 이로 인해 TestScripts 총 파일 수: **86개** (커버리지 향상용 err 3개 + 핫픽스 검증 1개 + 기존 82개)

---

## 3. 파일별 커버리지 Before / After

### 핵심 파이프라인 (변화 있는 파일)

| 파일 | Before | After | 변화 |
|------|--------|-------|------|
| `Environment.cpp` | 70% | **92%** | **+22%** |
| `InterpreterFactory/InterpreterFactory.cpp` | 57% | **71%** | **+14%** |
| `Interpreter.cpp` | 83% | **85%** | +2% |
| `EnvironmentTest.cpp` | 95% | **96%** | +1% |
| `InterpreterFactoryTest.cpp` | (없음) | **100%** | 신규 |

### 핵심 파이프라인 (변화 없는 파일)

| 파일 | 커버리지 | 측정 라인 수 |
|------|---------|------------|
| `Lexer.h` | 100% | 6 |
| `Lexer.cpp` | 100% | 97 |
| `Token.h` | 100% | 8 |
| `Parser.h` | 100% | 2 |
| `Expr.h` | 100% | 54 |
| `CheckerUnit/CheckerError.h` | 100% | 2 |
| `InterpreterFactory/InterpreterRunner.h` | 100% | 1 |
| `Parser.cpp` | 99% | 202 |
| `CheckerUnit/CheckerUnit.cpp` | 96% | 167 |
| `Callable.cpp` | 89% | 19 |
| `CheckerUnit/CheckEvalOperation.cpp` | 88% | 69 |
| `Callable.h` | 87% | 8 |
| `InterpreterFactory/FileRunner.cpp` | 84% | 33 |
| `Stmt.h` | 84% | 51 |
| `Environment.h` | 80% | 5 |
| `Interpreter.h` | 66% | 3 |
| `main.cpp` | 65% | 23 |

---

## 4. 현재 커버리지 상세 (After 기준 전체)

### 4-1. 프로젝트 소스 — 100%

| 파일 | 라인 수 |
|------|--------|
| `Token.h` | 8 |
| `Parser.h` | 2 |
| `Lexer.h` | 6 |
| `Lexer.cpp` | 97 |
| `Expr.h` | 54 |
| `CheckerUnit/CheckerError.h` | 2 |
| `InterpreterFactory/InterpreterRunner.h` | 1 |
| `InterpreterFactoryTest.cpp` | 6 |

### 4-2. 프로젝트 소스 — 90%~99%

| 파일 | 커버리지 | 라인 수 | 미커버 라인 수 |
|------|---------|--------|------------|
| `Parser.cpp` | 99% | 202 | ~2 |
| `CheckerUnit/CheckerUnit.cpp` | 96% | 167 | ~7 |
| `Environment.cpp` | 92% | 27 | ~2 |

### 4-3. 프로젝트 소스 — 80%~89%

| 파일 | 커버리지 | 라인 수 | 미커버 라인 수 |
|------|---------|--------|------------|
| `Callable.cpp` | 89% | 19 | ~2 |
| `CheckerUnit/CheckEvalOperation.cpp` | 88% | 69 | ~8 |
| `Callable.h` | 87% | 8 | ~1 |
| `Interpreter.cpp` | 85% | 255 | ~38 |
| `InterpreterFactory/FileRunner.cpp` | 84% | 33 | ~5 |
| `Stmt.h` | 84% | 51 | ~8 |
| `Environment.h` | 80% | 5 | ~1 |

### 4-4. 프로젝트 소스 — 79% 이하 (자동화 한계)

| 파일 | 커버리지 | 라인 수 | 미커버 원인 |
|------|---------|--------|-----------|
| `InterpreterFactory/InterpreterFactory.cpp` | 71% | 7 | REPL/DEBUG case — 대화형 |
| `Interpreter.h` | 66% | 3 | 일부 인라인 미실행 |
| `main.cpp` | 65% | 23 | REPL/debug 분기 — 대화형 |

### 4-5. 프로젝트 소스 — 0% (자동화 불가)

| 파일 | 라인 수 | 이유 |
|------|--------|------|
| `InterpreterFactory/ReplRunner.cpp` | 46 | REPL 대화형 입력 필요 |
| `InterpreterFactory/DebugRunner.cpp` | 22 | debug 모드 대화형 |
| `Debugger/Debugger.cpp` | 255 | debug 세션 필요 |
| `Debugger/DebugCommandParser.cpp` | 24 | debug 세션 필요 |
| `Debugger/Commands/FlowCommands.cpp` | 3 | debug 세션 필요 |
| `Debugger/Commands/BreakCommands.cpp` | 3 | debug 세션 필요 |
| `Debugger/Commands/WatchCommands.cpp` | 4 | debug 세션 필요 |

---

## 5. 자동화 한계 — 더 이상 올릴 수 없는 이유

### Dead Code (도달 불가)

| 위치 | 경로 | 이유 |
|------|------|------|
| `Interpreter.cpp:148` | `visitUnaryExpr` default | MINUS/BANG만 존재, 새 연산자 없으면 진입 불가 |
| `Interpreter.cpp:205-208` | `visitBinaryExpr` default | 모든 이항 연산자 처리됨 |
| `CheckEvalOperation.cpp:82` | `evaluateArithmeticOp` default | 내부 helper, 외부에서 정해진 타입만 전달 |
| `CheckEvalOperation.cpp:98` | `evaluateComparisonOp` default | 동일 |
| `CheckEvalOperation.cpp:113` | `evaluateEqualityOp` — ArrayPtr/CallablePtr | 정적 분석 대상 아님 |

### 대화형 모드 (stdin 파이프 없이 불가)

| 위치 | 이유 |
|------|------|
| `main.cpp` REPL 분기 | `ReplRunner::run()` → `std::getline(std::cin)` 대기 |
| `main.cpp` debug 분기 | `Debugger::runDebugLoop()` → 명령어 입력 대기 |
| `Debugger/*` 전체 | 동일 |

---

## 6. 테스트 파일 전체 커버리지 (참고)

| 파일 | 커버리지 | 라인 수 |
|------|---------|--------|
| `LexerTestFixture.cpp` | 100% | 115 |
| `InterpreterTest.cpp` | 100% | 89 |
| `ExecutorFuncArrayTest.cpp` | 100% | 85 |
| `InterpreterStmtTest.cpp` | 100% | 48 |
| `TestHelpers.h` | 100% | 48 |
| `InterpreterFactoryTest.cpp` | 100% | 6 |
| `ParserTest.cpp` | 99% | 821 |
| `ParseErrorTest.cpp` | 97% | 45 |
| `CheckerUnit/UnitTest/CheckerUnitTest.cpp` | 96% | 397 |
| `EnvironmentTest.cpp` | 96% | 53 |

---

## 7. 결론

| 구분 | 수치 |
|------|------|
| **자동화 도달 가능 최대 커버리지** (Dead code·대화형 제외) | **약 90~91%** |
| **현재 달성 커버리지** (핵심 파이프라인) | **약 88%** |
| **남은 자동화 가능 갭** | **약 2~3%** (일부 Stmt.h, FileRunner 세부 분기) |
| **구조적 한계** (Debugger + REPL) | 약 8% — stdin 모킹 없이 불가 |

> HTML 상세 리포트: `coverage_report/index.html`  
> 커버리지 재측정: `pwsh -File run_coverage.ps1`
