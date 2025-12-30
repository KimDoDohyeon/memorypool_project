cmake_minimum_required(VERSION 3.10)

# 프로젝트 설정
project(MempoolTest LANGUAGES CXX)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-O3 -Wall)

# -------------------------------------------------------------
# 1. 파이썬 인터프리터 찾기
# -------------------------------------------------------------
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# -------------------------------------------------------------
# 2. 파이썬 스크립트 실행 규칙 정의 (Custom Command)
# -------------------------------------------------------------
# 목표: build 폴더 안에 allocation_log.csv를 만드는 것
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/allocation_log.csv
    
    # 실행할 명령: python3 allocation_log_generater.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/allocation_log_generater.py
    
    # 중요: 파이썬이 실행될 위치를 build 폴더로 지정해야, csv가 그 자리에 생김
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    
    # 의존성: 파이썬 파일이 수정되면 다시 실행함
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/allocation_log_generater.py
    
    COMMENT "Running Python script to generate allocation_log.csv..."
)

# -------------------------------------------------------------
# 3. 데이터 생성 타겟 추가
# -------------------------------------------------------------
# 위에서 정의한 command를 실제로 수행할 'gen_data'라는 이름표(Target)를 만듦
add_custom_target(gen_data DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/allocation_log.csv)

# -------------------------------------------------------------
# 4. 실행 파일 빌드 및 의존성 연결
# -------------------------------------------------------------
add_executable(alloc_test simpleMempool.cpp)

# "alloc_test를 빌드하기 전에 무조건 gen_data(csv 생성)부터 끝내라"고 지시
add_dependencies(alloc_test gen_data)