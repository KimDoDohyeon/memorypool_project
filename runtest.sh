#!/bin/bash

# ==========================================
# 1. 초기 설정 및 OS 감지
# ==========================================
PROJECT_ROOT=$(pwd)
RESULT_FILE="$PROJECT_ROOT/result_performance.txt"
BUILD_DIR="build"
EXECUTABLE_NAME="alloc_test"

# OS 확인 (Linux vs Darwin)
OS_NAME=$(uname)

echo "--- Memory Allocator Performance Test (Stack vs Queue) ---" > "$RESULT_FILE"
echo "Date: $(date)" >> "$RESULT_FILE"
echo "OS: $OS_NAME" >> "$RESULT_FILE"
echo "" >> "$RESULT_FILE"

# 합계 변수 (소수점 연산을 위해 초기값 0)
TOTAL_BASIC=0
TOTAL_STACK=0
TOTAL_QUEUE=0
ITERATIONS=10

# ==========================================
# 2. CPU Governor 설정 (Linux Only)
# ==========================================
if [[ "$OS_NAME" == "Linux" ]]; then
    echo "Checking CPU governor settings..."
    if [ -d /sys/devices/system/cpu ]; then
        # 모든 코어에 대해 performance 모드 설정 시도
        for cpu in /sys/devices/system/cpu/cpu[0-9]*; do
            govfile="$cpu/cpufreq/scaling_governor"
            if [ -w "$govfile" ]; then
                echo performance > "$govfile" 2>/dev/null || true
            else
                # 권한 없으면 sudo 시도 (비밀번호 입력 필요할 수 있음)
                sudo bash -c "echo performance > $govfile" 2>/dev/null || true
            fi
        done
        echo "CPU governor set to 'performance' (if permitted)."
    fi
fi

echo "Starting $ITERATIONS iterations of testing..."
echo "Target Executable: $BUILD_DIR/$EXECUTABLE_NAME"

# 빌드 폴더 내 실행 파일 확인
if [ ! -f "$BUILD_DIR/$EXECUTABLE_NAME" ]; then
    echo "Error: 실행 파일이 없습니다. (cd build && make)를 먼저 하세요."
    exit 1
fi

# ==========================================
# 3. 테스트 루프 (프로세스 격리 적용)
# ==========================================
for ((i=1; i<=ITERATIONS; i++))
do
    echo -n "Iteration $i... "

    # ------------------------------------------------
    # [A] 명령어 프리픽스 설정 (CPU Pinning)
    # ------------------------------------------------
    #if [[ "$OS_NAME" == "Linux" ]]; then
    #    # Linux: 0번 코어 고정
    #    CMD_PREFIX="taskset -c 0"
    #else
    #    CMD_PREFIX=""
    #fi

    # ------------------------------------------------
    # [B] 프로세스 격리 실행 (각각 별도 프로세스로 실행)
    # ------------------------------------------------
    
    # 1. Basic Allocator 실행
    OUTPUT_BASIC=$(cd $BUILD_DIR && $CMD_PREFIX ./$EXECUTABLE_NAME --mode=basic 2>&1)
    BASIC_TIME=$(echo "$OUTPUT_BASIC" | grep "\[1\]" | awk -F': ' '{print $2}' | sed 's/ms//')

    # 2. Stack Allocator 실행
    OUTPUT_STACK=$(cd $BUILD_DIR && $CMD_PREFIX ./$EXECUTABLE_NAME --mode=stack 2>&1)
    STACK_TIME=$(echo "$OUTPUT_STACK" | grep "\[2\]" | awk -F': ' '{print $2}' | sed 's/ms//')

    # 3. Queue Allocator 실행
    OUTPUT_QUEUE=$(cd $BUILD_DIR && $CMD_PREFIX ./$EXECUTABLE_NAME --mode=queue 2>&1)
    QUEUE_TIME=$(echo "$OUTPUT_QUEUE" | grep "\[3\]" | awk -F': ' '{print $2}' | sed 's/ms//')

    # 에러 체크 (값이 비어있으면 문제 발생)
    if [ -z "$BASIC_TIME" ] || [ -z "$STACK_TIME" ] || [ -z "$QUEUE_TIME" ]; then
        echo "Error: Failed to parse output. Make sure C++ code supports --mode flags."
        exit 1
    fi

    echo "Basic: ${BASIC_TIME}ms, Stack: ${STACK_TIME}ms, Queue: ${QUEUE_TIME}ms"
    echo "Iteration $i - Basic: $BASIC_TIME, Stack: $STACK_TIME, Queue: $QUEUE_TIME" >> "$RESULT_FILE"
    
    # ------------------------------------------------
    # [C] Floating Point 합산 (awk 사용)
    # ------------------------------------------------
    TOTAL_BASIC=$(awk "BEGIN {print $TOTAL_BASIC + $BASIC_TIME}")
    TOTAL_STACK=$(awk "BEGIN {print $TOTAL_STACK + $STACK_TIME}")
    TOTAL_QUEUE=$(awk "BEGIN {print $TOTAL_QUEUE + $QUEUE_TIME}")
done

# ==========================================
# 4. 최종 결과 계산 (Floating Point Support)
# ==========================================

# 0으로 나누기 방지
if [ $(awk "BEGIN {print ($TOTAL_BASIC == 0)}") -eq 1 ]; then TOTAL_BASIC=1; fi

# 평균 계산
AVG_BASIC=$(awk "BEGIN {printf \"%.2f\", $TOTAL_BASIC / $ITERATIONS}")
AVG_STACK=$(awk "BEGIN {printf \"%.2f\", $TOTAL_STACK / $ITERATIONS}")
AVG_QUEUE=$(awk "BEGIN {printf \"%.2f\", $TOTAL_QUEUE / $ITERATIONS}")

# 성능 향상률 계산
IMP_STACK=$(awk "BEGIN {printf \"%.2f\", ($AVG_BASIC - $AVG_STACK) / $AVG_BASIC * 100}")
IMP_QUEUE=$(awk "BEGIN {printf \"%.2f\", ($AVG_BASIC - $AVG_QUEUE) / $AVG_BASIC * 100}")

echo "" >> "$RESULT_FILE"
echo "------------------------------------------" >> "$RESULT_FILE"
echo "Final Average Results ($ITERATIONS runs):" >> "$RESULT_FILE"
echo "1. Basic Allocator:       $AVG_BASIC ms" >> "$RESULT_FILE"
echo "2. Mempool (Stack/LIFO):  $AVG_STACK ms (Speedup: $IMP_STACK%)" >> "$RESULT_FILE"
echo "3. Mempool (Queue/FIFO):  $AVG_QUEUE ms (Speedup: $IMP_QUEUE%)" >> "$RESULT_FILE"
echo "------------------------------------------" >> "$RESULT_FILE"

echo ""
echo "Test complete. Results saved in $RESULT_FILE"
echo "--- Summary ---"
tail -n 6 "$RESULT_FILE"