// 포토캐논
#include <string.h>
#include <iostream>

class Photon_Cannon {
  int hp, shield;
  int coord_x, coord_y;
  int damage;

 public:
  Photon_Cannon(int x, int y);
  Photon_Cannon(const Photon_Cannon& pc);
  Photon_Cannon();
  Photon_Cannon& operator=(const Photon_Cannon& pc);

  void show_status();
};
Photon_Cannon::Photon_Cannon(const Photon_Cannon& pc) {
  std::cout << "복사 생성자 호출 !" << std::endl;
  hp = pc.hp;
  shield = pc.shield;
  coord_x = pc.coord_x;
  coord_y = pc.coord_y;
  damage = pc.damage;
}
Photon_Cannon::Photon_Cannon(int x, int y) {
  std::cout << "생성자 호출 !" << std::endl;
  hp = shield = 100;
  coord_x = x;
  coord_y = y;
  damage = 20;
}
Photon_Cannon::Photon_Cannon()
{
    std::cout << "디폴트 생성자 호출 !" << std::endl;
  hp = 0;
  shield = 0;
  coord_x = 0;
  coord_y = 0;
  damage = 0;
}
// 반환형은 Photon_Cannon& (참조)
Photon_Cannon& Photon_Cannon::operator=(const Photon_Cannon& pc) {
    std::cout << "대입 연산자 호출 !" << std::endl;
    
    // 자기 자신 대입 체크
    if (this == &pc) return *this;

    // 멤버 변수 값 복사
    hp = pc.hp;
    shield = pc.shield;
    coord_x = pc.coord_x;
    coord_y = pc.coord_y;
    damage = pc.damage;

    // 자기 자신 반환
    return *this;
}
void Photon_Cannon::show_status() {
  std::cout << "Photon Cannon " << std::endl;
  std::cout << " Location : ( " << coord_x << " , " << coord_y << " ) "
            << std::endl;
  std::cout << " HP : " << hp << std::endl;
}
int main() {
  Photon_Cannon pc1(3, 3);
  Photon_Cannon pc2(pc1);
  Photon_Cannon pc3;
  pc3 = pc2;

  pc1.show_status();
  pc2.show_status();
  pc3.show_status();
}