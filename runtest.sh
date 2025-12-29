#!/bin/bash

# 결과 저장 파일 초기화
RESULT_FILE="result_stack.txt"
echo "--- Memory Allocator Performance Test ---" > $RESULT_FILE
echo "Date: $(date)" >> $RESULT_FILE
echo "" >> $RESULT_FILE

# 합계를 저장할 변수 (소수점 계산을 위해 초기값 0)
TOTAL_BASIC=0
TOTAL_MEMPOOL=0
ITERATIONS=10

echo "Starting $ITERATIONS iterations of testing..."

for ((i=1; i<=ITERATIONS; i++))
do
    echo "Iteration $i..."
    
    # 프로그램 실행 및 결과 캡처
    # 결과가 "Basic: 123.45 ms" 형태라고 가정하고 숫자만 추출
    OUTPUT=$(./alloc_test)
    
    # grep과 sed를 이용해 숫자값만 파싱
    BASIC_TIME=$(echo "$OUTPUT" | grep "basic allocator" | sed 's/[^0-9.]*//g')
    MEMPOOL_TIME=$(echo "$OUTPUT" | grep "mempool allocator" | sed 's/[^0-9.]*//g')
    
    echo "  Basic: $BASIC_TIME ms, Mempool: $MEMPOOL_TIME ms"
    echo "Iteration $i - Basic: $BASIC_TIME ms, Mempool: $MEMPOOL_TIME ms" >> $RESULT_FILE
    
    # 합계 누적 (bc를 이용한 실수 연산)
    TOTAL_BASIC=$((TOTAL_BASIC + BASIC_TIME))
    TOTAL_MEMPOOL=$((TOTAL_MEMPOOL + MEMPOOL_TIME))
done

# 평균 계산
AVG_BASIC=$(awk "BEGIN {print $TOTAL_BASIC / $ITERATIONS}")
AVG_MEMPOOL=$(awk "BEGIN {print $TOTAL_MEMPOOL / $ITERATIONS}")
ENHANCEMENT = =$(awk "BEGIN {print (1 - $AVG_MEMPOOL / $AVG_BASIC) * 100 }")

# 최종 결과 저장
echo "" >> $RESULT_FILE
echo "------------------------------------------" >> $RESULT_FILE
echo "Final Average Results ($ITERATIONS runs):" >> $RESULT_FILE
echo "Average Basic Allocator: $AVG_BASIC ms" >> $RESULT_FILE
echo "Average Mempool Allocator: $AVG_MEMPOOL ms" >> $RESULT_FILE
echo "—————————————————————" >> $RESULT_FILE

echo "Test complete. Results saved in $RESULT_FILE"
cat $RESULT_FILE | tail -n 5