# memorypool_project

## 프로젝트 설명

memorypool_project는 메모리 풀을 구현한 프로젝트입니다. 메모리 할당의 효율성을 높이기 위해 C++로 메모리 풀 클래스를 제공하며, 할당 로그 생성, 성능 테스트 등의 기능을 포함합니다.

메모리 풀은 기본 할당자(malloc/free), 스택 기반(LIFO), 큐 기반(FIFO) 세 가지 방식을 지원하며, 성능 비교를 위한 테스트를 제공합니다.

## 파일 구조

- `simpleMempool.cpp`: 메모리 풀 구현 및 테스트 실행 파일
- `allocation_log_generator.py`: 할당 로그(CSV) 생성 Python 스크립트
- `CMakeLists.txt`: CMake 빌드 설정 파일
- `runtest.sh`: 성능 테스트 실행 스크립트 (10회 반복 테스트)
- `result_performance.txt`: 성능 테스트 결과 파일
- `build/`: 빌드 디렉토리 (CMake로 생성)

## 요구 사항

- C++17 지원 컴파일러 (g++, clang 등)
- CMake 3.10 이상
- Python 3.x
- Linux 환경 (CPU Governor 설정을 위해)

## 빌드 방법

1. 프로젝트 루트 디렉토리로 이동:
   ```bash
   cd memorypool_project
   ```

2. 빌드 디렉토리 생성 및 이동:
   ```bash
   mkdir build
   cd build
   ```

3. CMake 구성:
   ```bash
   cmake ..
   ```

4. 빌드 실행:
   ```bash
   make
   ```
   - 병렬 빌드: `make -j$(nproc)`

빌드 완료 후 `build/alloc_test` 실행 파일이 생성됩니다.

## 테스트 실행

프로젝트 루트에서 다음 명령어로 성능 테스트를 실행합니다:

```bash
./runtest.sh
```

### 테스트 설명
- **CPU Governor 설정**: Linux에서 CPU를 'performance' 모드로 설정 시도 (권한 필요)
- **로그 생성**: 각 테스트 반복마다 `allocation_log.csv`를 생성하여 메모리 할당 패턴 시뮬레이션
- **성능 측정**: 기본 할당자, 스택 메모리 풀, 큐 메모리 풀의 할당/해제 시간을 측정
- **반복 횟수**: 10회 반복하여 평균 성능 계산
- **결과**: `result_performance.txt`에 평균 시간과 성능 향상률 기록

### 테스트 결과 해석
결과 파일 예시:
```
Final Average Results (10 runs):
1. Basic Allocator:       150.5 ms
2. Mempool (Stack/LIFO):  120.3 ms (Speedup: 25.1%)
3. Mempool (Queue/FIFO):  118.7 ms (Speedup: 26.8%)
```

- **Speedup**: 기본 할당자 대비 성능 향상률
- 스택/큐 메모리 풀이 일반적으로 더 빠름

## 사용법

### 직접 실행
빌드 후:
```bash
./build/alloc_test --mode=basic    # 기본 할당자 테스트
./build/alloc_test --mode=stack    # 스택 메모리 풀 테스트
./build/alloc_test --mode=queue    # 큐 메모리 풀 테스트
```

### 로그 파일 재생성
```bash
python3 allocation_log_generator.py
```

## 주의사항

- CPU Governor 변경은 root 권한 필요 (쿠버네티스 환경에서는 제한될 수 있음)
- 테스트는 CPU 0번 코어에 고정 실행 (`taskset`)
- 메모리 사용량이 많을 수 있으므로 충분한 RAM 확보

## 라이선스

(라이선스 정보 추가 예정)
