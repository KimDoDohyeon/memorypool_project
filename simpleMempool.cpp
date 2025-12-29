#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <chrono>

#define MAX_ENTRIES 1<<20

typedef struct entry{
    int64_t index;
    int64_t obj_id;
    char op;
} entry;

class basic {
private:
    std::unordered_map<int, int64_t*> allocated_map;    // 자료형 체크
    
public:
    void alloc(int obj_id){
        int64_t *ptr = (int64_t*)malloc(sizeof(int64_t));
        allocated_map[obj_id] = ptr;
    }
    void free(int obj_id){
        int64_t *ptr = allocated_map[obj_id];
        allocated_map.erase(obj_id);
        ::free(ptr);
    }

};

class mempool {
private:
    std::vector<void*> mempool_vector;
    std::unordered_map<int, int64_t*> allocated_map;

public:
    void alloc(int64_t obj_id){
        if(!mempool_vector.empty()){
            int64_t *ptr = (int64_t*)mempool_vector.back();
            mempool_vector.pop_back();
            allocated_map[obj_id] = ptr;
        }
        else{
            int64_t *ptr = (int64_t*)malloc(sizeof(int64_t));
            allocated_map[obj_id] = ptr;
        }
    }
    void free(int64_t obj_id){
        int64_t *ptr = allocated_map[obj_id];
        allocated_map.erase(obj_id);
        mempool_vector.push_back(ptr);
    }
    
};
// 파일을 열어서 각 command를 entry로 갖는 array 생성
std::vector<entry> readFile(const std::string& filename){
    std::vector<entry> entries; // -> heap 영역을 가리킨다고?
    std::ifstream file(filename); // <- 무슨 문법이지?

    if (!file.is_open()) {
        std::cerr << "error: " << filename << std::endl;
        return entries;
    }

    std::string line;
    while (std::getline(file, line)) { // -> 무슨 함수지?
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string temp;
        entry e;

        // -> try, catch 문법?
        if (std::getline(ss, temp, ',')) {
            try {
                e.op = temp[0];
                if (std::getline(ss, temp)) { 
                    e.obj_id = std::stoll(temp); 
                    entries.push_back(e);
                }
            } catch (...) {
                continue;
            }
        }
    }

    file.close();
    return entries;
}

int main(){

    printf("start reading\n");
    fflush(stdout);
    std::vector<entry> entries = readFile("allocation_log.csv");

    basic basic_allocator;
    mempool mempool_allocator;

    // basic memory allocator
    printf("start basic allocator\n");
    auto start_basic = std::chrono::high_resolution_clock::now();

    for (const auto &e : entries)
    {
        if(e.op == 'a'){
            basic_allocator.alloc(e.obj_id);
        }
        else{
            basic_allocator.free(e.obj_id);
        }
   }

   auto end_basic = std::chrono::high_resolution_clock::now();
   auto ms_basic = std::chrono::duration_cast<std::chrono::milliseconds>(end_basic - start_basic);
   std::cout << "basic allocator: " << ms_basic.count() << "ms" << std::endl;

   //mempool memory allocator
   printf("start mempool allocator\n");
   auto start_mempool = std::chrono::high_resolution_clock::now();

   for (const auto& e : entries) {
        if(e.op == 'a'){
            mempool_allocator.alloc(e.obj_id);
        }
        else{
            mempool_allocator.free(e.obj_id);
        }
   }

   auto end_mempool = std::chrono::high_resolution_clock::now();
   auto ms_mempool = std::chrono::duration_cast<std::chrono::milliseconds>(end_mempool - start_mempool);
   std::cout << "mempool allocator: " << ms_mempool.count() << "ms" << std::endl;   
}