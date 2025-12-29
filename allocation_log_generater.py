import csv
import random
import sys

csv.field_size_limit(sys.maxsize)

def generate_allocation_log(max_id, pool_limit):
    """
    
    """
    filename = 'allocation_log.csv'

    current_alloc_id = 1
    active_objs = []

    total_ops = 0

    print(f"Generating log... MAX_ID={max_id}, POOL_LIMIT={pool_limit}")

    with open(filename, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(['op', 'obj_id'])

        while current_alloc_id <= max_id or len(active_objs) > 0:
            is_alloc = False

            if len(active_objs) >= pool_limit:
                is_alloc = False
            elif len(active_objs) == 0:
                is_alloc = True
            elif current_alloc_id > max_id:
                is_alloc = False
            else:
                ALLOC_PROB = 0.50
                if random.random() < ALLOC_PROB:
                    is_alloc = True
                else:
                    is_alloc = False
            
            if is_alloc:
                writer.writerow(['a', current_alloc_id])
                active_objs.append(current_alloc_id)
                current_alloc_id += 1
            else:
                rand_idx = random.randrange(len(active_objs))
                target_id = active_objs[rand_idx]
                writer.writerow(['f', target_id])
                # swap and pop, O(1) 삭제 트릭 -> pop(random_idx)는 O(N)이 걸릴 수 있음 ?
                active_objs[rand_idx] = active_objs[-1]
                active_objs.pop()
            
            total_ops += 1
            if total_ops % 1_000_000 == 0:
                print(f"Processed {total_ops} events... (Current Pool Size: {len(active_objs)})")
    
    print(f"완료. 파일명: {filename}")
    print(f"총 이벤트 수: {total_ops}")

def check_valid(max_id, pool_limit):
    filename = 'allocation_log.csv'
    
    # 현재 메모리 할당된 목록들
    active_objs = set()
    max_peak_usage = 0

    print(f"Validating {filename}...")

    try:
        with open(filename, 'r', newline='', encoding='utf-8') as f:
            reader = csv.reader(f)

            header = next(reader, None)
            if not header:
                print("Error: File is empty.")
                return
            
            # csv 파일에서 몇번째 줄인지 line_num에 저장, ['a', 1] 같은 실제 내용을 row에 저장 (row[0]='a', row[1]=1)
            for line_num, row in enumerate(reader, start=2):
                if not row: continue

                op = row[0]
                try:
                    obj_id = int(row[1])
                except ValueError:
                    print(f"[Line {line_num}] Error: obj_id is not integer '{row[1]}'")
                    return
                
                if op == 'a':
                    # double allocation check
                    if obj_id in active_objs:
                        print(f"[Line {line_num}] Error! Double allocatioin for obj_id {obj_id}")
                        return

                    active_objs.add(obj_id)

                    current_size = len(active_objs)
                    if current_size > max_peak_usage:
                        max_peak_usage = current_size
                    if current_size > pool_limit:
                        print(f"[Line {line_num}] FAIL! Pool size {current_size} exceeded limit {pool_limit}")
                        return
                
                elif op == 'f':
                    # double free check
                    if obj_id not in active_objs:
                        print(f"[Line {line_num}] Error! Double free (or free before alloc) for obj_id {obj_id}")
                        return
                    active_objs.remove(obj_id)
                
                if line_num % 1_000_000 == 0:
                    print(f"Checked {line_num} lines... (Current Pool: {len(active_objs)})")

    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        return
    
    if len(active_objs) == 0:
        print("\nSUCCESS: Validation Passed!")
        print(f"Max Peak Pool Size was: {max_peak_usage} (Limit: {pool_limit})")
    else:
        print(f"\nWARNING: Log ended but {len(active_objs)} objects are still allocated (Memory Leak).")
        print(f"Leaked IDs (first 10): {list(active_objs[:10])}...")




if __name__ == "__main__":
    MAX_ID = 1 << 26
    POOL_LIMIT = 1 << 20
    generate_allocation_log(MAX_ID, POOL_LIMIT)
    check_valid(MAX_ID, POOL_LIMIT)
