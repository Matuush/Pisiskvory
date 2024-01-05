#include <iostream>
#include <vector>

enum tile {fr, p1, p2};

struct pos {
   int y, x;
   pos(int py, int px) : y(py), x(px) {}
   pos() {}
   pos operator*(int n) {
      return {y*n, x*n};
   }
   pos operator+(pos p){
      return {p.y+y, p.x+x};
   }
   void operator+=(pos p){
      y += p.y;
      x += p.x;
   }
};

struct Game {
   tile won = fr;
   int size;
   std::vector<std::vector<tile>> map;
   Game(int s) : size(s){
      std::vector<tile> row(size, fr);
      map = std::vector<std::vector<tile>>(size, row);
   }
   tile at(pos p) {
      return map[p.y][p.x];
   }
   bool valid(pos p) {
      return p.x >= 0 && p.x < size && p.y >= 0 && p.y < size;
   }
   int place(pos p, tile t) {
      if(!valid(p)) return 1;
      if(at(p) != fr) return 2;
      map[p.y][p.x] = t;
      return 0;
   }
   bool checkWin(pos p) {
      int win_count = size < 5 ? size : 5;
      tile t0 = at(p);
      if(t0 == fr) return false;
      for(int vy = -1; vy <= 1; vy++) {
         for(int vx = -1; vx <= 1; vx++) {
            if(vy == 0 && vx == 0) continue;;
            pos v = {vy,vx};
            bool win = true;
            for(int n = 1; n < 5; n++) {
               if(!valid(v*n+p) || at(v*n+p) != t0) {
                  win = false;
                  break;
               }
            }
            if(win) {
               won = t0;
               return true;
            }
         }
      }
      return false;
   }
   void print() {
       for(auto r : map) {
         for(tile t : r) {
            std::cout << ((t == fr) ? '.' : (t == p1) ? 'O' : 'X');
         }
         std::cout << std::endl;
      }
   }
};

int main(int argc, char** argv) {
   Game g(15);
   pos p(0,0);
      std::cout << "check\n";
   g.print();
   for(int t = 0; !g.checkWin(p); t++) {
      tile ti = t%2 == 0 ? p1 : p2;
      while(true){
         int y,x; std::cin >> y >> x;
         pos pl(y,x);
         if(!g.place(pl,ti)) {
            p = pl;
            break;
         }
      }
      g.print();
   }

   return 0;
}
