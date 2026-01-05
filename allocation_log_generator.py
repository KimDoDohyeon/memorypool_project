import csv
import random
import sys

# ==========================================
# [설정] 동작 모드 선택
# True  : 톱니바퀴 패턴 (Fill/Drain 반복) -> Cache Locality 테스트에 유리 (권장)
# False : 50:50 랜덤 패턴 (Ping-Pong) -> Pool 깊이가 얕아짐
# ==========================================
USE_SAWTOOTH_MODE = False 

csv.field_size_limit(sys.maxsize)

def generate_allocation_log(max_id, pool_limit):
    filename = 'allocation_log.csv'
    current_alloc_id = 1
    active_objs = []
    total_ops = 0

    # Sawtooth 모드용 상태 변수 ('fill' or 'drain')
    phase = 'fill' 

    mode_str = "Sawtooth (Fill/Drain)" if USE_SAWTOOTH_MODE else "Random (50:50)"
    print(f"Generating log... [{mode_str}]")
    print(f"MAX_ID={max_id}, POOL_LIMIT={pool_limit}")

    with open(filename, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(['op', 'obj_id'])

        while current_alloc_id <= max_id or len(active_objs) > 0:
            
            # -------------------------------------------------
            # 1. 확률(ALLOC_PROB) 결정 로직
            # -------------------------------------------------
            ALLOC_PROB = 0.5 # 기본값

            if USE_SAWTOOTH_MODE:
                # [Sawtooth Mode]
                # 현재 활성 객체 수에 따라 채우기(fill)/비우기(drain) 모드 전환
                if len(active_objs) >= pool_limit * 0.95:
                    phase = 'drain' # 95% 차면 비우기 시작
                elif len(active_objs) <= pool_limit * 0.05 and current_alloc_id <= max_id:
                    phase = 'fill'  # 5% 이하로 떨어지면 다시 채우기

                # 모드에 따른 확률 할당
                if phase == 'fill':
                    ALLOC_PROB = 0.95  # 강력하게 채움
                else:
                    ALLOC_PROB = 0.05  # 강력하게 비움
            else:
                # [Random Mode]
                ALLOC_PROB = 0.50      # 단순 50:50

            # -------------------------------------------------
            # 2. 실제 동작 결정 (강제 조건 포함)
            # -------------------------------------------------
            is_alloc = False

            if len(active_objs) >= pool_limit:
                is_alloc = False # 꽉 찼으면 무조건 해제
            elif len(active_objs) == 0:
                is_alloc = True  # 비었으면 무조건 할당
            elif current_alloc_id > max_id:
                is_alloc = False # ID 다 썼으면 남은거 해제
            else:
                # 위에서 정한 확률 적용
                if random.random() < ALLOC_PROB:
                    is_alloc = True
                else:
                    is_alloc = False
            
            # -------------------------------------------------
            # 3. CSV 기록 및 상태 업데이트
            # -------------------------------------------------
            if is_alloc:
                writer.writerow(['a', current_alloc_id])
                active_objs.append(current_alloc_id)
                current_alloc_id += 1
            else:
                rand_idx = random.randrange(len(active_objs))
                target_id = active_objs[rand_idx]
                writer.writerow(['f', target_id])
                
                # O(1) 삭제 트릭 (Swap and Pop)
                active_objs[rand_idx] = active_objs[-1]
                active_objs.pop()
            
            total_ops += 1
            if total_ops % 1_000_000 == 0:
                status_msg = f"Phase: {phase}" if USE_SAWTOOTH_MODE else "Phase: Random"
                print(f"Processed {total_ops} events... (Active: {len(active_objs)}, {status_msg})")
    
    print(f"완료. 파일명: {filename}")
    print(f"총 이벤트 수: {total_ops}")

if __name__ == "__main__":
    # 테스트 규모 설정
    MAX_ID = 1 << 26     # 총 생성할 ID 개수 (약 6700만)
    POOL_LIMIT = 1 << 20 # 풀 최대 크기 (약 100만 객체)
    
    generate_allocation_log(MAX_ID, POOL_LIMIT)