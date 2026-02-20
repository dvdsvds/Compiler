# DYC 컴파일러 문서

## 목차
1. [프로젝트 개요](#프로젝트-개요)
2. [컴파일러 동작 흐름](#컴파일러-동작-흐름)
3. [주요 파일 설명](#주요-파일-설명)
4. [동시성 기능 상세 설명](#동시성-기능-상세-설명)
5. [주요 기능](#주요-기능)

---

## 프로젝트 개요

이 프로젝트는 **"dyc"**라는 이름의 컴파일러입니다. `.dy` 확장자를 가진 소스 코드 파일을 입력받아 `.dvs` 확장자를 가진 어셈블리 코드 파일로 변환합니다.

### 컴파일 과정의 주요 단계

1. **Lexical Analysis (어휘 분석)** - 소스 코드를 토큰으로 분해
2. **Parsing (구문 분석)** - 토큰 시퀀스를 추상 구문 트리(AST)로 변환
3. **Semantic Analysis (의미 분석)** - 타입 검사 및 의미 검증
4. **IR Generation (중간 표현 생성)** - AST를 중간 표현(IR)으로 변환
5. **Assembly Emission (어셈블리 코드 생성)** - IR을 어셈블리 코드로 변환

---

## 컴파일러 동작 흐름

```
소스 코드 (.dy)
    ↓
[Lexer] → 토큰 시퀀스
    ↓
[Parser] → AST (추상 구문 트리)
    ↓
[SemanticAnalyzer] → 의미 검사 (오류 발견 시 중단)
    ↓
[IRBuilder] → IR (중간 표현)
    ↓
[AssemblyEmitter] → 어셈블리 코드 (.dvs)
```

### 실행 방법

```bash
dyc <input.dy>
```

입력 파일: `example.dy`  
출력 파일: `example.dvs`

---

## 주요 파일 설명

### 1. **src/main.cpp** - 메인 진입점
- **역할**: 컴파일러의 진입점 및 전체 파이프라인 조율
- **주요 동작**:
  1. 명령줄 인자로 `.dy` 파일 입력 받기
  2. 소스 파일 읽기
  3. **Lexer**로 토큰화
  4. **Parser**로 AST 생성
  5. Import 처리 (모듈 로드 및 병합)
  6. **SemanticAnalyzer**로 의미 분석
  7. **IRBuilder**로 IR 생성
  8. **AssemblyEmitter**로 어셈블리 코드 출력
  9. `.dvs` 파일 생성

### 2. **include/token.hpp** - 토큰 정의
- **역할**: 언어의 모든 토큰 타입 정의
- **주요 구성**:
  - `Token` enum: 모든 키워드, 연산자, 구분자 정의
  - `TokenData` 구조체: 토큰 타입, 값, 위치 정보(줄/열) 저장
- **지원 타입**:
  - 정수: `S8`, `S16`, `S32` (부호 있는)
  - 부호 없는 정수: `US8`, `US16`, `US32`
  - 불린: `BOOL`
  - 배열: `S8_ARRAY`, `S16_ARRAY`, `S32_ARRAY` 등
  - 포인터: `PS8`, `PS16`, `PS32` 등
  - 특수 키워드: `SEND`, `RECV`, `IN`, `OUT` (동시성 관련)

### 3. **include/lexer.hpp & src/lexer.cpp** - 어휘 분석기
- **역할**: 소스 코드를 토큰 시퀀스로 변환
- **주요 기능**:
  - 공백 및 주석 건너뛰기
  - 숫자 리터럴 인식
  - 문자열 및 문자 리터럴 인식
  - 식별자 및 키워드 구분
  - 위치 정보(줄/열) 추적
- **동작 방식**:
  - 소스 코드를 한 글자씩 읽으면서 토큰으로 분해
  - 키워드 맵을 사용하여 식별자를 키워드로 변환

### 4. **include/parser.hpp & src/parser.cpp** - 파서
- **역할**: 토큰 시퀀스를 추상 구문 트리(AST)로 변환
- **파싱 방식**: 재귀 하강 파싱(Recursive Descent Parsing)
- **주요 기능**:
  - 표현식 파싱 (우선순위 처리)
  - 문장 파싱 (변수 선언, 제어문 등)
  - 함수 선언 파싱
  - 구조체 선언 파싱
  - Import 처리
- **표현식 우선순위**:
  - 논리 연산 (`AND`, `OR`)
  - 비트 연산 (`BIT_AND`, `BIT_OR`, `BIT_XOR`)
  - 비교 연산 (`EQ`, `NE`, `LT`, `GT` 등)
  - 산술 연산 (`PLUS`, `MINUS`, `STAR`, `SLASH` 등)

### 5. **include/ast.hpp & src/ast.cpp** - 추상 구문 트리
- **역할**: 프로그램의 구조를 트리 형태로 표현
- **노드 계층 구조**:
  - `ASTNode`: 기본 노드 클래스 (위치 정보 포함)
  - `Expr`: 표현식 노드
    - `LiteralExpr`: 리터럴 값 (숫자, 문자열 등)
    - `VariableExpr`: 변수 참조
    - `BinaryExpr`: 이항 연산
    - `UnaryExpr`: 단항 연산
    - `CallExpr`: 함수 호출
    - `ArrayAccessExpr`: 배열 접근
    - `InExpr`, `OutExpr`: 동시성 표현식
    - `ReferenceExpr`, `DereferenceExpr`: 포인터 연산
    - `ArrayExpr`: 배열 리터럴
    - `MemberAccessExpr`: 구조체 멤버 접근
  - `Stmt`: 문장 노드
    - `VarDeclStmt`: 변수 선언
    - `AssignStmt`: 할당
    - `IfStmt`: 조건문
    - `LoopStmt`: 반복문
    - `ReturnStmt`: 반환
    - `BreakStmt`, `ContinueStmt`: 루프 제어
    - `SendStmt`, `RecvStmt`: 동시성 문장
    - `BlockStmt`: 블록
    - `PrintStmt`: 출력
  - `FunctionDecl`: 함수 선언
  - `StructDecl`: 구조체 선언
  - `ImportDecl`: Import 선언
  - `Program`: 프로그램 루트 노드
- **Visitor 패턴**: AST 순회를 위한 인터페이스 제공

### 6. **include/visitor.hpp** - Visitor 패턴
- **역할**: AST 순회를 위한 인터페이스 정의
- **기능**: 각 노드 타입별 `visit` 메서드 선언
- **사용처**: `SemanticAnalyzer`, `IRBuilder`가 이를 구현하여 AST를 순회

### 7. **include/symbol.hpp & src/symbol.cpp** - 심볼
- **역할**: 변수, 함수, 파라미터 등의 심볼 정보 저장
- **주요 정보**:
  - 심볼 종류: `VARIABLE`, `FUNCTION`, `PARAMETER`
  - 타입 정보
  - 스코프 레벨
  - OUT 변수 여부
  - 주소 취득 여부 (포인터 연산에 사용됨)

### 8. **include/scope.hpp & src/scope.cpp** - 스코프
- **역할**: 스코프 계층 구조 관리
- **기능**:
  - 부모 스코프 참조
  - 심볼 삽입 및 조회
  - 스코프 레벨 추적

### 9. **include/symbolTable.hpp & src/symbolTable.cpp** - 심볼 테이블
- **역할**: 스코프 스택 관리 및 심볼 조회
- **주요 기능**:
  - `enter_scope()`: 새 스코프 진입
  - `exit_scope()`: 현재 스코프 종료
  - `insert()`: 심볼 삽입
  - `lookup()`: 심볼 조회 (현재 및 상위 스코프 검색)
  - `lookup_current_scope()`: 현재 스코프에서만 조회

### 10. **include/semanticAnalyzer.hpp & src/semanticAnalyzer.cpp** - 의미 분석기
- **역할**: AST를 순회하며 의미 검사 수행
- **주요 검사 항목**:
  - 변수/함수 선언 및 사용 검증
  - 타입 일치 검사
  - 함수 시그니처 일치 검사
  - SEND/RECV 쌍 검증
  - OUT 변수 소비 검증
  - Break/Continue가 루프 내에서 사용되는지 검증
  - Return 타입 검증
- **오류 처리**:
  - 오류 수집 및 저장
  - 오류 출력 기능
  - 오류 타입 분류

### 11. **include/IR.hpp & src/IR.cpp** - 중간 표현(IR)
- **역할**: 컴파일러의 중간 표현 정의
- **주요 구성 요소**:
  - `IROpcode`: IR 연산 코드
    - 산술: `ADD`, `SUB`, `MUL`, `DIV`, `MOD`, `NEG`
    - 비트: `AND`, `OR`, `XOR`, `NOT`, `SHL`, `SHR`, `SAR`
    - 비교: `EQ`, `NE`, `LT`, `LE`, `GT`, `GE`
    - 메모리: `LOAD`, `STORE`, `ALLOCA`
    - 제어: `LABEL`, `JUMP`, `BRANCH`
    - 함수: `CALL`, `CALL_PTR`, `RETURN`
    - 동시성: `SEND`, `RECV`, `OUT`, `IN`
    - 기타: `COPY`, `PHI`, `PRINT`
  - `Operand`: 피연산자
    - 가상 레지스터 (`VIRTUAL_REG`)
    - 상수 (`CONSTANT`)
    - 레이블 (`LABEL`)
    - 글로벌 변수 (`GLOBAL`)
    - 스택 슬롯 (`STACK_SLOT`)
  - `IRInstruction`: IR 명령어
  - `BasicBlock`: 기본 블록 (제어 흐름 그래프의 노드)
  - `IRFunction`: 함수 (파라미터, 반환 타입, 기본 블록들)
  - `IRModule`: 모듈 (함수들, 글로벌 변수들, 문자열 리터럴들)

### 12. **include/IRBuilder.hpp & src/IRBuilder.cpp** - IR 생성기
- **역할**: AST를 IR로 변환
- **주요 기능**:
  - Visitor 패턴으로 AST 순회
  - 가상 레지스터 할당
  - 기본 블록 생성 및 관리
  - 제어 흐름 처리 (if, loop)
  - 함수 호출 처리
  - SEND/RECV 처리
  - 구조체 멤버 접근 처리
  - PHI 노드 생성 (SSA 형태)

### 13. **include/assemblyEmitter.hpp & src/assemblyEmitter.cpp** - 어셈블리 생성기
- **역할**: IR을 어셈블리 코드로 변환
- **주요 기능**:
  - 레지스터 할당 (가상 레지스터 → 물리 레지스터)
  - 스필(Spill) 처리 (레지스터가 부족할 때 스택 사용)
  - PHI 제거 (SSA 형태를 일반 코드로 변환)
  - 함수 프롤로그/에필로그 생성
  - 호출자 저장 레지스터 보존
  - SEND/RECV 주소 할당

---

## 동시성 기능 상세 설명

이 컴파일러는 함수 간 통신과 동시성 처리를 위한 특수한 기능들을 제공합니다.

### 1. OUT/IN 메커니즘 - 함수 간 값 전달

**OUT 변수**: 함수가 값을 "생산"하여 스택에 저장  
**IN 표현식**: 다른 함수가 그 값을 "소비"하여 읽음

#### 사용 예시

```dy
// 함수 A: 값을 생산
func producer() {
    S32 x = 42;
    out(x);  // x를 스택에 저장 (OUT 변수로 표시)
}

// 함수 B: 값을 소비
func consumer() {
    S32 y = in(x);  // 스택에서 x 값을 읽음 (한 번만 읽을 수 있음)
    print(y);  // 42 출력
}
```

#### 동작 방식

- **`out(variableName)`**: 
  - 변수 값을 스택에 저장
  - `offset`을 사용하여 스택에 값을 쌓음
  - 변수를 OUT 변수로 표시
  
- **`in(variableName)`**: 
  - 스택에서 값을 읽음
  - `offset`을 증가시켜 다음 값 읽기
  - OUT 변수는 한 번만 소비 가능 (semanticAnalyzer에서 검증)

#### 어셈블리 코드 변환

```assembly
// OUT 처리
csrr r30, SP          ; 스택 포인터 읽기
storew rs1, offset(r30)  ; 값을 스택에 저장
offset -= 4            ; 스택 포인터 조정

// IN 처리
offset += 4            ; 스택 포인터 조정
csrr r_temp, SP        ; 스택 포인터 읽기
loadw rd, offset(r_temp)  ; 스택에서 값 읽기
```

### 2. SEND/RECV 메커니즘 - 비동기 메시지 전달

**SEND**: 변수 값을 특정 메모리 영역에 저장  
**RECV**: 특정 메모리 영역에서 값을 읽음

#### 사용 예시

```dy
// 함수 A: 값을 전송
func sender() {
    S32 data = 100;
    send(data);  // data를 공유 메모리 영역에 저장
}

// 함수 B: 값을 수신
func receiver() {
    S32 received;
    recv(received);  // 공유 메모리 영역에서 값 읽기
    print(received);  // 100 출력
}
```

#### 동작 방식

- **`send(variableName)`**: 
  - 변수 값을 `next_send_recv_addr` 주소에 저장
  - 주소를 4바이트씩 증가시킴 (스택처럼 쌓임)
  
- **`recv(variableName)`**: 
  - `next_send_recv_addr`에서 값을 읽음
  - 주소를 4바이트씩 감소시킴 (스택처럼 꺼냄)
  
- **검증**: SemanticAnalyzer가 모든 SEND에 대응하는 RECV가 있는지 검증

#### 어셈블리 코드 변환

```assembly
// SEND 처리
mov r_temp, next_send_recv_addr  ; 주소 로드
storew value_reg, 0(r_temp)      ; 값 저장
next_send_recv_addr += 4          ; 주소 증가

// RECV 처리
next_send_recv_addr -= 4          ; 주소 감소
mov r_temp, next_send_recv_addr   ; 주소 로드
loadw rd, 0(r_temp)               ; 값 읽기
```

### 3. 사용 시나리오

#### 시나리오 1: 파이프라인 처리

```dy
// 생산자-소비자 패턴
func producer() {
    S32 i = 0;
    loop (i < 10) {
        out(i);  // 값을 생산
        i = i + 1;
    }
}

func consumer() {
    loop (true) {
        S32 value = in(i);  // 값을 소비
        print(value);
        if (value == 9) break;
    }
}
```

#### 시나리오 2: 작업 분산

```dy
// 여러 작업자에게 작업 분배
func master() {
    S32 task1 = 1;
    S32 task2 = 2;
    S32 task3 = 3;
    
    send(task1);  // 작업자 1에게
    send(task2);  // 작업자 2에게
    send(task3);  // 작업자 3에게
}

func worker1() {
    S32 task;
    recv(task);  // 작업 수신
    // task 처리...
}

func worker2() {
    S32 task;
    recv(task);  // 작업 수신
    // task 처리...
}
```

#### 시나리오 3: 이벤트 기반 통신

```dy
// 이벤트 발생 및 처리
func eventGenerator() {
    S32 event = 1;
    send(event);  // 이벤트 전송
}

func eventHandler() {
    S32 evt;
    recv(evt);  // 이벤트 수신
    if (evt == 1) {
        // 이벤트 처리
    }
}
```

### 4. 메모리 레이아웃

```
[스택 영역 - OUT/IN용]
    ↓ (offset 감소)
[OUT 값들...]
    ↑ (offset 증가)
[IN 값들...]

[공유 메모리 영역 - SEND/RECV용]
    ↓ (next_send_recv_addr 증가)
[SEND 값들...]
    ↑ (next_send_recv_addr 감소)
[RECV 값들...]
```

### 5. 검증 규칙

SemanticAnalyzer가 다음 규칙들을 검증합니다:

1. **OUT 변수 소비 규칙**: OUT 변수는 한 번만 소비 가능
   - 같은 OUT 변수를 두 번 `in()`으로 읽으면 오류

2. **SEND/RECV 쌍 검증**: 
   - 모든 SEND에 대응하는 RECV가 있어야 함
   - 모든 RECV에 대응하는 SEND가 있어야 함

3. **IN 사용 규칙**: 
   - `in()`은 OUT 변수에만 사용 가능
   - OUT 변수가 아닌 변수에 `in()` 사용 시 오류

4. **타입 일치**: 
   - SEND와 RECV의 변수 타입이 일치해야 함

---

## 주요 기능

### 1. 타입 시스템
- **정수 타입**: `S8`, `S16`, `S32` (부호 있는 정수)
- **부호 없는 정수**: `US8`, `US16`, `US32`
- **불린**: `BOOL`
- **배열**: 모든 기본 타입의 배열 지원
- **포인터**: 모든 타입의 포인터 지원
- **구조체**: 사용자 정의 구조체 타입

### 2. 구조체
- 사용자 정의 타입 정의
- 멤버 변수 접근 (`.` 연산자)
- 포인터를 통한 멤버 접근 (`->` 연산자)
- 구조체 리터럴

### 3. 함수
- 파라미터 전달
- 반환값
- 함수 포인터 지원
- 재귀 호출 지원

### 4. 제어 흐름
- **조건문**: `if`/`else`
- **반복문**: `loop` (C의 for 루프와 유사)
- **분기**: `switch`/`case`/`default`
- **루프 제어**: `break`, `continue`

### 5. 동시성 기능
- **OUT/IN**: 함수 간 값 전달
- **SEND/RECV**: 비동기 메시지 전달
- 의미 분석을 통한 안전성 보장

### 6. 모듈 시스템
- `import` 키워드를 통한 모듈 로드
- 선택적 import (특정 심볼만 import)
- 모듈 병합 기능

### 7. 메모리 관리
- 포인터 연산
- 참조 (`&`) 및 역참조 (`*`)
- 주소 취득 추적
- 스택 기반 메모리 할당

### 8. 기타 기능
- `print` 문을 통한 출력
- 인라인 어셈블리 (`asm`) 지원
- 문자열 리터럴
- 주석 처리

---

## 컴파일러 아키텍처 요약

이 컴파일러는 전통적인 컴파일러 구조를 따릅니다:

1. **프론트엔드**: Lexer → Parser → SemanticAnalyzer
2. **미들엔드**: IRBuilder (AST → IR)
3. **백엔드**: AssemblyEmitter (IR → Assembly)

각 단계는 명확히 분리되어 있으며, Visitor 패턴을 통해 확장 가능한 구조로 설계되었습니다.