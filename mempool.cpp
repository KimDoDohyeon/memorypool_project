#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>
#include <cstdint> // int64_t 사용을 위해 필요

// CSV에서 읽은 하나의 명령을 저장할 구조체
struct LogEntry {
    char op;        // 'a' (allocation) or 'f' (free)
    int64_t obj_id; // Object ID
};

// CSV 파일을 읽어서 LogEntry 리스트로 반환하는 함수
std::vector<LogEntry> read_allocation_log(const std::string& filename) {
    std::vector<LogEntry> logs;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return logs;
    }

    // 첫 줄(헤더) 건너뛰기: op,obj_id
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string segment;
        LogEntry entry;

        // 1. 쉼표(,)를 기준으로 분리
        // 첫 번째: op (a 또는 f)
        if (std::getline(ss, segment, ',')) {
            entry.op = segment[0];
        }
        
        // 두 번째: obj_id
        if (std::getline(ss, segment, ',')) {
            entry.obj_id = std::stoll(segment); // string to long long
        }

        logs.push_back(entry);
    }

    file.close();
    std::cout << "Loaded " << logs.size() << " operations from " << filename << std::endl;
    return logs;
}

int main() {
    // 1. 로그 파일 읽어오기
    std::string filename = "allocation_log.csv";
    std::vector<LogEntry> operations = read_allocation_log(filename);

    // 2. 자료구조 초기화 (제시해주신 내용)
    // free_list only used in mempool version (메모리 풀 방식일 때 미리 할당된 청크들을 보관)
    std::queue<int64_t*> mempool; 
    
    // <obj_id, pointer> used in both versions (할당된 객체 추적용)
    std::unordered_map<int64_t, int64_t*> allocated_map; 

    // TODO: (Mempool 방식이라면) 여기서 미리 메모리 풀을 초기화(Pre-allocation) 하는 코드가 필요할 수 있습니다.
    // ex) for(...) mempool.push(new int64_t);


    // 3. 로그 순회하며 시뮬레이션 수행
    for (const auto& log : operations) {
        if (log.op == 'a') {
            // ==========================================
            // [TODO 1] 할당 (Allocation) 로직 구현
            // ==========================================
            std::cout << "Allocate Obj ID: " << log.obj_id << std::endl;

            // 1) mempool이 비어있는지 확인 또는 시스템 할당(new)
            // 2) 할당받은 주소를 allocated_map[log.obj_id]에 저장
            
        } 
        else if (log.op == 'f') {
            // ==========================================
            // [TODO 2] 해제 (Free) 로직 구현
            // ==========================================
            std::cout << "Free Obj ID: " << log.obj_id << std::endl;

            // 1) allocated_map에서 log.obj_id로 주소 찾기
            // 2) 해당 주소를 mempool로 반환하거나 시스템 해제(delete)
            // 3) map에서 해당 항목 제거
        }
    }

    // TODO: 종료 전 남은 메모리 정리(Cleanup)

    return 0;
}