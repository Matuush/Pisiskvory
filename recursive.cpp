#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <deque>
#include <string>
#include <raylib.h>

using std::deque;
using std::string;
using std::array;
using std::vector;
using std::min;
using std::cin;
using std::cout;

enum tile {fr = 0, p1 = 1, p2 = 2, tie = 3};
tile act(int t) { return (t % 2) ? p2 : p1; }
tile pas(int t) { return (t % 2) ? p1 : p2; }
#define point(pos) {(float)pos.x, (float)pos.y}

#define FPS 15
#define SCREEN 800
#define LINE_W 2

#define IMG_COUNT 2
array<Texture2D, IMG_COUNT> textures;
void prepareTextures() {
   const char* paths[] = {"img/circle.png", "img/cross.png"};
   for(int i = 0; i < IMG_COUNT; i++) {
      Image img;
      img = LoadImage(paths[i]);
      textures[i] = LoadTextureFromImage(img);
   }
}
void renderImage(tile t, Rectangle dst, Color color = WHITE) {
   if(t == fr || t == tie) return;
   Texture tex = textures[t-1];
   Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
   DrawTexturePro(textures[t-1], src, dst, {0,0}, 0, color);
}

struct pos {
   int x, y;
   pos() : x(-1), y(-1) {}
   pos(int px, int py) : x(px), y(py) {}
   pos(Vector2 v) : x(v.x), y(v.y) {}
   pos normalize() {
      if(x >= 3) x = 2;
      else if(x < 0) x = 0;
      if(y >= 3) y = 2;
      else if(y < 0) y = 0;
      return *this;
   }
   pos operator/(int a) {return {x/a, y/a};};
   pos operator%(int a) {return {x%a, y%a};};
   int to_int() { return 3*y + x; }
   deque<int> to_deque(int depth, int screenSize) {
      deque<int> r = {};
      pos pp = pos(x,y);
      int pwr = 1;
      for(int i = 1; i <= depth; i++) {
         pwr *= 3;
         pos ppos = (pp/(screenSize/pwr));
         if(ppos.x > 2 || ppos.y > 2) return {};
         r.push_back(ppos.to_int());
         pp = pp%(screenSize/pwr);
      }
      return r;
   }
};

#define OKAY -1
#define CHANGE -2
struct Block {
   tile t = fr;
   bool leaf = false;
   Rectangle target;
   array<Block*, 9> sons;

   Block() {}
   Block(int depth, Rectangle trg) {
      target = trg;
      int x = trg.x, y = trg.y, w = trg.width, h = trg.height;
      if(depth == 0) leaf = true;
      else
         for(int i = 0; i < 9; i++) 
            sons[i] = new Block(depth-1, 
               {float(int(x + w/3 * (i%3))), float(int(y + h/3 * (i/3))), float(int(w/3)), float(int(h/3))});
   }
   int change(tile pl) {
      t = pl;
      if(pl == fr)
         return OKAY;
      return CHANGE;
   }
   bool playable(deque<int> v) {
      if(leaf && t == fr) return true;
      if(t != fr || v.size() == 0) return false;
      int son = v.front();
      v.pop_front();
      return sons[son]->playable(v);
   }
   int play(deque<int> v, tile pl) {
      if(t != fr) return v.size();
      if(leaf) return change(pl);
      if(v.size() == 0) return OKAY;

      int son = v.front();
      v.erase(v.begin());
      int rv = sons[son]->play(v, pl);

      if(rv == CHANGE) return change(check_win());
      return rv;
   }
   tile check_win() {
      tile d1 = sons[0]->t, d2 = sons[2]->t;
      for(int k = 0; k < 3; k++) {
         tile row = sons[k*3]->t, col = sons[k%3]->t;
         for(int i = 0; i < 3; i++) {
            if(sons[k*3 + i]->t != row) row = fr;
            if(sons[k%3 + i*3]->t != col) col = fr;
         }
         if(row != fr) return row;
         if(col != fr) return col;
         if(sons[k*4]->t != sons[0]->t) d1 = fr;
         if(sons[(k+1)*2]->t != sons[2]->t) d2 = fr;
      }
      if(d1 != fr) return d1;
      if(d2 != fr) return d2;

      bool is_tie = true;
      for(Block* s : sons) 
         if(s->t == fr) is_tie = false;
      if(is_tie) return tie;

      return fr;
   }
   void render(int depth) {
      if(!leaf) DrawRectangleLinesEx(target, depth, BLACK);
      renderImage(t, target);
      if(leaf || (t != fr && t != tie)) return;

      for(int i = 0; i < 9; i++) 
         sons[i]->render(depth-1);
   }
   Rectangle getRect(deque<int> v) {
      if(v.size() == 0) return target;
      else {
         int index = v.front();
         v.pop_front();
         return sons[index]->getRect(v);
      }
   }
   void updateRect(Rectangle trg) {
      target = trg;
      int x = trg.x, y = trg.y, w = trg.width, h = trg.height;
      if(leaf) return;
      for(int i = 0; i < 9; i++) 
         sons[i]->updateRect({
            float(int(x + w/3 * (i%3))), 
            float(int(y + h/3 * (i/3))), 
            float(int(w/3)), 
            float(int(h/3))
         });
   }

   void save(vector<tile> *r) {
      if(leaf) r->push_back(t);
      else
         for(Block* son : sons)
            son->save(r);
   }
   void load_state(vector<tile> state, int pos) {
      if(leaf) t = state[pos];
      else {
         for(int i = 0; i < 9; i++)
            sons[i]->load_state(state, pos*9+i);
         t = check_win();
      }
   }
   void undo(deque<int> &prev) {
      // TODO
   }
};

void save_state(Block* root, int depth, int time, deque<int> &turn) {
   vector<tile> state = {};
   root->save(&state);
   string filename; std::cin >> filename;
   std::ofstream outfile("saves/" + filename);
   outfile << depth << "\n" << time << "\n" << turn.size() << "\n";
   for(int play : turn)
      outfile << play << std::endl;
   for(tile t : state) 
      outfile << int(t) << std::endl;
}
void load_state(Block** root, deque<int> &turn, int &depth, int &time, char* argv) {
   depth = atoi(argv);
   if(depth)
      *root = new Block(depth, {0, 0, SCREEN, SCREEN});
   else {
      string path = "saves/" + string(argv);
      std::ifstream infile(path);
      infile >> depth >> time;
      int K; infile >> K;
      for(int k = 0; k < K; k++) {
         int play; infile >> play;
         turn.push_back(play);
         cout << play;
      }
      vector<tile> state = {};
      long long pw = 1;
      for(int i = 0; i < depth; i++) pw *= 9;
      for(int i = 0; i < pw; i++) {
         int next; infile >> next;
         state.push_back((tile)next);
      }
      *root = new Block(depth, {0, 0, SCREEN, SCREEN});
      (*root)->load_state(state, 0);
   }
}

void update(int &time, deque<int> &turn, Block* root, Rectangle &dst) {
   time++;
   turn.erase(turn.begin());
   int rval_2 = root->play(turn, pas(time));
   for(int i = 0; i <= rval_2; i++) 
      turn.pop_back();
   dst = root->getRect(turn);
}
void undo(Block *root, deque<int> &prev, Rectangle &dst, int &time) {
   if(prev.size() == 0) return;
   time--;
   root->undo(prev);
   // TODO
}
bool valid(Block* root, Rectangle &dst, deque<int> &turn) {
   return CheckCollisionRecs(root->getRect(turn), dst) && root->playable(turn);
}
void render_all(Block* root, int time, Rectangle dst, int depth, deque<int> highlight = {}) {
   BeginDrawing();
   ClearBackground(RAYWHITE);
   root->render(depth);
   if(valid(root, dst, highlight))
      renderImage(act(time), root->getRect(highlight), BLACK);
   DrawRectangleLinesEx(dst, 3*LINE_W, act(time+1) == p2 ? BLUE : RED);
   EndDrawing();
}
void prepareScreen(Block* root, int time, Rectangle dst, int depth) {
   SetConfigFlags(FLAG_WINDOW_RESIZABLE);
   InitWindow(SCREEN, SCREEN, "Rekurze");
   SetTargetFPS(FPS);
   prepareTextures();
   render_all(root, time, dst, depth);
}


int main(int argc, char** argv) {
   deque<int> turn = {};
   int time = 0, depth;
   Block* root = nullptr;

   int screenSize = SCREEN;

   if(argc <= 1) {
      cin >> depth;
      root = new Block(depth, {0, 0, SCREEN, SCREEN});
   }
   else load_state(&root, turn, depth, time, argv[1]);

   Rectangle dst = root->getRect(turn);

   prepareScreen(root, time, dst, depth);
   

   bool end = false;
   deque<int> pturn = {};
   while(!WindowShouldClose()) {
      // Update screen size
      screenSize = min(GetScreenWidth(), GetScreenHeight());
      root->updateRect({0, 0, (float)screenSize, (float)screenSize});
      dst = root->getRect(pturn);

      // Move
      Vector2 touch = GetMousePosition();
      turn = pos(touch).to_deque(depth, screenSize);
      if(IsMouseButtonPressed(0) && CheckCollisionPointRec(touch, dst) && valid(root, dst, turn)) {
         int rval = root->play(turn, act(time));
         if(rval == CHANGE) end = true;
         else if(rval == OKAY) {
            update(time, turn, root, dst);
            pturn = turn;
         }
      }

      // Key presses
      if(IsKeyPressed(KEY_X)) save_state(root, depth, time, turn), end = true;
      else if(IsKeyPressed(KEY_Z)); //TODO undo
      else if(IsKeyPressed(KEY_P)) TakeScreenshot("screenshot.png");
      
      // Rendering
      render_all(root, time, dst, depth, turn);

      if(end) break;
   }

   WaitTime(10);
   CloseWindow();
   return 0;
}
