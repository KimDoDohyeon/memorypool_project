#include <iostream>
#include <unordered_map>
#include <vector>
#include <deque>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <climits>
#include <cstdlib>

#define MAX_POOL_SIZE (1 << 20)

typedef struct entry
{
    int64_t index;
    int64_t obj_id;
    char op;
} entry;

// 1. 기본 할당자 (malloc/free 직접 호출)
class BasicAllocator
{
private:
    std::unordered_map<int64_t, int64_t *> allocated_map; // unordered_map 초고속 검색 장부 (hash map)
                                                          // key: int, value: int64_t*

public:
    BasicAllocator()
    {
        allocated_map.reserve(MAX_POOL_SIZE);
    }
    void alloc(int64_t obj_id)
    {
        int64_t *ptr = (int64_t *)std::malloc(sizeof(int64_t));
        allocated_map[obj_id] = ptr;
    }
    void free(int64_t obj_id)
    {
        int64_t *ptr = allocated_map[obj_id];
        allocated_map.erase(obj_id);
        std::free(ptr);
    }
};

// 2. stack 방식 메모리 풀 (LIFO) - std::vector 사용
class MempoolStack
{
private:
    std::vector<void *> pool; // vector는 뒤로 넣고 뒤에서 빼면 스택처럼 동작
    std::unordered_map<int64_t, int64_t *> allocated_map;

public:
    MempoolStack()
    {
        pool.reserve(MAX_POOL_SIZE);
        allocated_map.reserve(MAX_POOL_SIZE);
    }
    void alloc(int64_t obj_id)
    {
        if (!pool.empty())
        {
            int64_t *ptr = (int64_t *)pool.back();
            pool.pop_back();
            allocated_map[obj_id] = ptr;
        }
        else
        {
            int64_t *ptr = (int64_t *)std::malloc(sizeof(int64_t));
            allocated_map[obj_id] = ptr;
        }
    }
    void free(int64_t obj_id)
    {
        int64_t *ptr = allocated_map[obj_id];
        allocated_map.erase(obj_id);
        pool.push_back(ptr);
    }
};

// queue 방식 메모리 풀 (FIFO) - std::deque 사용
class MempoolQueue
{
private:
    std::deque<void *> pool;
    std::unordered_map<int64_t, int64_t *> allocated_map;

public:
    MempoolQueue()
    {
        allocated_map.reserve(MAX_POOL_SIZE);
    }
    void alloc(int64_t obj_id)
    {
        if (!pool.empty())
        {
            int64_t *ptr = (int64_t *)pool.front();
            pool.pop_front();
            allocated_map[obj_id] = ptr;
        }
        else
        {
            int64_t *ptr = (int64_t *)std::malloc(sizeof(int64_t));
            allocated_map[obj_id] = ptr;
        }
    }
    void free(int64_t obj_id)
    {
        int64_t *ptr = allocated_map[obj_id];
        allocated_map.erase(obj_id);
        pool.push_back(ptr);
    }
};

// [NEW] 원형 큐 (Circular Queue) 방식
// std::vector를 사용하지만 논리적으로는 Queue(FIFO)로 동작함
class MempoolCircularQueue
{
private:
    std::vector<void *> pool; // 데이터를 담을 그릇 (Vector 사용!)
    std::unordered_map<int64_t, int64_t *> allocated_map;
    
    size_t front_idx; // 큐의 앞 (나가는 곳)
    size_t rear_idx;  // 큐의 뒤 (들어오는 곳)
    size_t count;     // 현재 들어있는 개수
    size_t capacity;  // 큐의 최대 크기

public:
    // 생성자: 미리 큰 방을 잡아둡니다. (Reserve 효과)
    MempoolCircularQueue() : front_idx(0), rear_idx(0), count(0), capacity(MAX_POOL_SIZE) {
        pool.resize(MAX_POOL_SIZE); // reserve가 아니라 resize로 실제 공간을 채워둡니다.
        allocated_map.reserve(MAX_POOL_SIZE);
    }

    void alloc(int64_t obj_id)
    {
        // 큐에 재활용할 메모리가 있으면 (FIFO: 앞에서 뺌)
        if (count > 0)
        {
            int64_t *ptr = (int64_t *)pool[front_idx];
            
            // front를 한 칸 전진 (원형으로 뱅글뱅글 돔)
            front_idx = (front_idx + 1) % capacity; 
            count--;

            allocated_map[obj_id] = ptr;
        }
        else
        {
            int64_t *ptr = (int64_t *)std::malloc(sizeof(int64_t));
            allocated_map[obj_id] = ptr;
        }
    }

    void free(int64_t obj_id)
    {
        int64_t *ptr = allocated_map[obj_id];
        allocated_map.erase(obj_id);

        // 큐 뒤에 반납 (FIFO: 뒤로 넣음)
        // 안전장치: 큐가 꽉 차지 않았을 때만 넣음 (이 시뮬레이션에선 넘칠 일 없음)
        if (count < capacity) {
            pool[rear_idx] = ptr;
            
            // rear를 한 칸 전진
            rear_idx = (rear_idx + 1) % capacity;
            count++;
        }
        // 만약 큐가 꽉 찼다면? 그냥 free 해버리거나 에러 처리 (여기선 생략)
    }
};

// 파일을 열어서 각 command를 entry로 갖는 array 생성
std::vector<entry> readFile(const std::string &filename)
{
    std::vector<entry> entries; // 이 변수 자체는 함수 안에 선언되었으므로 스택(stack) 영역에 생김,
                                // 그러나 entries 안의 데이터는 들어올 때마다 힙(heap) 영역에 데이터를 저장
    entries.reserve(1 << 26);
    std::ifstream file(filename); // 생성자를 이용한 파일 열기,
                                  // file이라는 변수가 태어날 때 파일을 열고 변수가 사라질 때 파일을 닫음

    if (!file.is_open())
    {
        std::cerr << "error: " << filename << std::endl;
        return entries;
    }

    std::string line;
    // 헤더 건너뛰기 (op, obj_id)
    std::getline(file, line);

    while (std::getline(file, line)) // file에서 한 줄 읽어서 (\n 만날 때까지) line 변수에 string 형식으로 넣음
                                     // 읽는 중에는 true이다가 파일 끝에 도달하면 false가 되어 while문이 멈춤
    {
        if (line.empty())
            continue;

        std::stringstream ss(line); // 문자열 line을 파일인 척 감싸는 도구, 이렇게 하면 getline을 똑같이 쓸 수 있음
        std::string temp;
        entry e;

        if (std::getline(ss, temp, ',')) // 파일인 척 하는 문자열 ss에서 읽는데, ','가 나올 때까지만 읽어서 temp에 넣음
        {
            try
            {
                e.op = temp[0];             // 'a' 또는 'f' 저장
                if (std::getline(ss, temp)) // 'a'나 'f' 다음의 숫자 읽음
                {
                    e.obj_id = std::stoll(temp); // string to long long, 문자열을 int64_t 숫자로 바꿈
                    entries.push_back(e);        // entries 벡터에 추가
                }
            }
            catch (...) // try 안의 코드에서 에러가 나면 catch 안의 코드 진행
            {
                continue;
            }
        }
    }

    // file.close();
    // file close는 굳이 안해도 됨. C++ 소멸자가 알아서 함.
    return entries;
}

// [함수화] 테스트 로직을 함수로 분리 (템플릿 사용하면 더 좋지만 단순하게 구현)
template <typename Allocator>
long long run_benchmark(Allocator &allocator, const std::vector<entry> &entries)
{
    auto start = std::chrono::high_resolution_clock::now();

    for (const auto &e : entries)
    {
        if (e.op == 'a')
            allocator.alloc(e.obj_id);
        else
            allocator.free(e.obj_id);
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main(int argc, char** argv)
{
    // 0. 모드 파싱 (기본값: "all")
    std::string mode = "all";
    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.find("--mode=") == 0) {
            mode = arg.substr(7); // "--mode=" 길이가 7이므로 그 뒤부터 읽음
        }
    }

    // 1. 데이터 로드 (모든 모드 공통)
    // std::cout << "Loading data for mode: " << mode << "..." << std::endl;
    std::vector<entry> entries = readFile("allocation_log.csv");

    // 2. 반복 횟수 설정
    const int TEST_ITERATIONS = 5;

    // ---------------------------------------------------------
    // TEST 1: Basic Allocator
    // ---------------------------------------------------------
    if (mode == "basic" || mode == "all") 
    {
        long long min_time = LLONG_MAX;

        // [Warm-up]
        {
            BasicAllocator warmup_alloc;
            run_benchmark(warmup_alloc, entries);
        }

        // [Actual Test]
        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            BasicAllocator allocator;
            long long time = run_benchmark(allocator, entries);
            if (time < min_time) min_time = time;
        }
        std::cout << "[1] Basic Allocator: " << min_time << "ms" << std::endl;
    }

    // ---------------------------------------------------------
    // TEST 2: Stack Mempool
    // ---------------------------------------------------------
    if (mode == "stack" || mode == "all") 
    {
        long long min_time = LLONG_MAX;

        // [Warm-up]
        {
            MempoolStack warmup_alloc;
            run_benchmark(warmup_alloc, entries);
        }

        // [Actual Test]
        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            MempoolStack allocator;
            long long time = run_benchmark(allocator, entries);
            if (time < min_time) min_time = time;
        }
        std::cout << "[2] Mempool (STACK): " << min_time << "ms" << std::endl;
    }

    // ---------------------------------------------------------
    // TEST 3: Queue Mempool (Circular Queue)
    // ---------------------------------------------------------
    if (mode == "queue" || mode == "all") 
    {
        long long min_time = LLONG_MAX;

        // [Warm-up]
        {
            MempoolCircularQueue warmup_alloc;
            run_benchmark(warmup_alloc, entries);
        }

        // [Actual Test]
        for (int i = 0; i < TEST_ITERATIONS; ++i)
        {
            MempoolCircularQueue allocator;
            long long time = run_benchmark(allocator, entries);
            if (time < min_time) min_time = time;
        }
        std::cout << "[3] Mempool (QUEUE): " << min_time << "ms" << std::endl;
    }

    return 0;
}