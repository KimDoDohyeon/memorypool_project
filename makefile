# 컴파일러 설정
CC = g++

# 컴파일 옵션
CFLAGS = -O3 -Wall -std=c++17

# 결과물 이름
TARGET = alloc_test

# 소스 파일 및 오브젝트 파일
SRCS = simpleMempool.cpp
OBJS = $(SRCS:.cpp=.o)

# 기본 빌드 규칙
all: $(TARGET)

# 링크 단계
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# 컴파일 단계
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 생성된 파일 삭제
clean:
	rm -f $(OBJS) $(TARGET)

# 실행 규칙
run: $(TARGET)
	./$(TARGET)